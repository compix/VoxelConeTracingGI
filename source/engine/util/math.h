#pragma once
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct HSV;
struct RGB;

struct Polar
{
    float radius;
    float elevation;
    float azimuth;

    Polar(float radius, float elevation, float azimuth)
        : radius(radius), elevation(elevation), azimuth(azimuth) { }
};

namespace math
{
    const float PI = 3.14159265359f;
    const float EPSILON = 0.0000001f;
    const float EPSILON5 = 10e-5f;
    const float TO_DEG = 180.f / PI;
    const float TO_RAD = PI / 180.f;

    const float PI2 = PI * 2.0f;
    const float PI4 = PI2 * 2.0f;
    const float PI_DIV_2 = PI * 0.5f;
    const float PI_DIV_4 = PI * 0.25f;

    void rotate(glm::vec2& p, const float& angle);

    inline float lerp(float start, float end, float t) { return (1 - t) * start + t * end; }

    RGB lerp(const RGB& start, const RGB& end, float t);
    HSV lerp(const HSV& start, const HSV& end, float t);
    glm::vec2 lerp(const glm::vec2& start, const glm::vec2& end, float t);

    inline float clamp(float v, float min, float max) { return v < min ? min : v > max ? max : v; }

    inline float smoothstep(float start, float end, float t) { return start + t * t * (3 - 2 * t) * (end - start); }

    RGB smoothstep(RGB start, RGB end, float t);
    HSV smoothstep(HSV start, HSV end, float t);
    glm::vec2 smoothstep(const glm::vec2& start, const glm::vec2& end, float t);

    inline float smootherstep(float start, float end, float t) { return start + t * t * t * (t * (t * 6 - 15) + 10) * (end - start); }

    inline float regress(float start, float end, float t) { return start + (sqrtf(t) * 2.f - t) * (end - start); }

    RGB regress(RGB start, RGB end, float t);
    HSV regress(HSV start, HSV end, float t);
    glm::vec2 regress(const glm::vec2& start, const glm::vec2& end, float t);

    inline float min(float v0, float v1, float v2) { return std::min(v0, std::min(v1, v2)); }

    inline float max(float v0, float v1, float v2) { return std::max(v0, std::max(v1, v2)); }

    inline float toRadians(float degree) { return degree * PI / 180.f; }

    inline float toDegrees(float radians) { return radians * 180.f / PI; }

    inline glm::vec3 toDegrees(const glm::vec3& angles) { return glm::vec3(toDegrees(angles.x), toDegrees(angles.y), toDegrees(angles.z)); }

    inline glm::vec3 toRadians(const glm::vec3& angles) { return glm::vec3(toRadians(angles.x), toRadians(angles.y), toRadians(angles.z)); }

    inline bool nearEq(float n1, float n2) { return std::abs(n1 - n2) <= EPSILON; }

    inline bool nearEq(float n1, float n2, float epsilon) { return std::abs(n1 - n2) <= epsilon; }

    inline bool nearEq(const glm::vec2& v0, const glm::vec2& v1, float epsilon = EPSILON) { return nearEq(v0.x, v1.x, epsilon) && nearEq(v0.y, v1.y, epsilon); }

    inline bool nearEq(const glm::vec3& v0, const glm::vec3& v1, float epsilon = EPSILON) { return nearEq(v0.x, v1.x, epsilon) && nearEq(v0.y, v1.y, epsilon) && nearEq(v0.z, v1.z, epsilon); }

    inline bool nearEq(const glm::vec4& v0, const glm::vec4& v1, float epsilon = EPSILON) { return nearEq(v0.x, v1.x, epsilon) && nearEq(v0.y, v1.y, epsilon) && nearEq(v0.z, v1.z, epsilon) && nearEq(v0.w, v1.w); }

    /**
    * Returns a rotation matrix to rotate vector f to vector t. 
    * Source: "Efficiently Building a Matrix to Rotate One Vector to Another" by Tomas M�ller and John F. Hughes
    */
    glm::mat3 rotateVectorToAnother(const glm::vec3& f, const glm::vec3& t);

    inline glm::mat4 scale(float scale) { return glm::scale(glm::vec3(scale, scale, scale)); }

    glm::mat4 rotationX(float angle);
    glm::mat4 rotationY(float angle);
    glm::mat4 rotationZ(float angle);

    inline glm::mat4 orthoLH(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        glm::mat4 result(1.f);
        result[0][0] = 2.f / (right - left);
        result[1][1] = 2.f / (top - bottom);
        result[2][2] = 2.f / (zFar - zNear);
        result[3][0] = (right + left) / (left - right);
        result[3][1] = (top + bottom) / (bottom - top);
        result[3][2] = (zFar + zNear) / (zNear - zFar);
        return result;
    }

    inline glm::mat4 identityMatrix()
    {
        return glm::mat4(
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationX90Deg()
    {
        return glm::mat4(1.f, 0.f, 0.f, 0.f,
                         0.f, 0, 1.f, 0.f,
                         0.f, -1.f, 0, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationY90Deg()
    {
        return glm::mat4(0.f, 0.f, -1.f, 0.f,
                         0.f, 1.f, 0.f, 0.f,
                         1.f, 0.f, 0.f, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationZ90Deg()
    {
        return glm::mat4(0.f, 1.f, 0.f, 0.f,
                         -1.f, 0.f, 0.f, 0.f,
                         0.f, 0.f, 1.f, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationXMinus90Deg()
    {
        return glm::mat4(1.f, 0.f, 0.f, 0.f,
                         0.f, 0, -1.f, 0.f,
                         0.f, 1.f, 0, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationYMinus90Deg()
    {
        return glm::mat4(0.f, 0.f, 1.f, 0.f,
                         0.f, 1.f, 0.f, 0.f,
                         -1.f, 0.f, 0.f, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationZMinus90Deg()
    {
        return glm::mat4(0.f, -1.f, 0.f, 0.f,
                         1.f, 0.f, 0.f, 0.f,
                         0.f, 0.f, 1.f, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    /**
    * Rotation by y radians about y axis followed by 
    * rotation by x radians about x axis followed by 
    * rotation by z radians about z axis.
    */
    glm::quat eulerYXZQuat(const glm::vec3& eulerAngles);

    /**
    * Rotation by yaw radians about y axis followed by
    * rotation by pitch radians about x axis followed by
    * rotation by roll radians about z axis.
    */
    glm::quat eulerYXZQuat(float yaw, float pitch, float roll);

    /**
    * Rotation by yaw radians about y axis followed by
    * rotation by pitch radians about x axis followed by
    * rotation by roll radians about z axis.
    */
    glm::quat yawPitchRoll(float yaw, float pitch, float roll);

    inline glm::vec3 eulerAngles(const glm::quat& q)
    {
        return glm::vec3(asinf(2.0f * (-q.z * q.y + q.x * q.w)),
                         atan2f(2.0f * (q.w * q.y + q.x * q.z), 1.0f - 2.0f * (q.y * q.y + q.x * q.x)),
                         atan2f(2.0f * (q.w * q.z + q.y * q.x), 1.0f - 2.0f * (q.z * q.z + q.x * q.x)));
    }

    inline glm::vec3 polarToVector(float radius, float elevation, float azimuth)
    {
        return glm::vec3(-radius * cosf(elevation) * sinf(azimuth),
                         radius * sinf(elevation),
                         radius * cosf(elevation) * cosf(azimuth));
    }

    inline Polar vectorToPolar(glm::vec3 v)
    {
        float r = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
        return Polar(r, glm::asin(v.y / r), -glm::atan(v.x, v.z));
    }
}
