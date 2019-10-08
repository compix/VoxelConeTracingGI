#include "math.h"
#include "colors.h"
#include <cmath>

void math::rotate(glm::vec2& p, const float& angle)
{
    const float x = p.x;
    const float c = cosf(angle);
    const float s = sinf(angle);

    p.x = x * c - p.y * s;
    p.y = p.y * c + x * s;
}

RGB math::lerp(const RGB& start, const RGB& end, float t)
{
    RGB result;
    result.r = math::lerp(start.r, end.r, t);
    result.g = math::lerp(start.g, end.g, t);
    result.b = math::lerp(start.b, end.b, t);

    return result;
}

HSV math::lerp(const HSV& start, const HSV& end, float t)
{
    HSV result;
    result.h = math::lerp(start.h, end.h, t);
    result.s = math::lerp(start.s, end.s, t);
    result.v = math::lerp(start.v, end.v, t);

    return result;
}

glm::vec2 math::lerp(const glm::vec2& start, const glm::vec2& end, float t)
{
    glm::vec2 result;
    result.x = math::lerp(start.x, end.x, t);
    result.y = math::lerp(start.y, end.y, t);

    return result;
}

RGB math::smoothstep(RGB start, RGB end, float t)
{
    RGB result;
    float m = t * t * (3 - 2 * t);
    result.r = start.r + m * (end.r - start.r);
    result.g = start.g + m * (end.g - start.g);
    result.b = start.b + m * (end.b - start.b);

    return result;
}

HSV math::smoothstep(HSV start, HSV end, float t)
{
    HSV result;
    float m = t * t * (3 - 2 * t);
    result.h = start.h + m * (end.h - start.h);
    result.s = start.s + m * (end.s - start.s);
    result.v = start.v + m * (end.v - start.v);

    return result;
}

glm::vec2 math::smoothstep(const glm::vec2& start, const glm::vec2& end, float t)
{
    glm::vec2 result;
    float m = t * t * (3 - 2 * t);
    result.x = start.x + m * (end.x - start.x);
    result.y = start.y + m * (end.y - start.y);

    return result;
}

RGB math::regress(RGB start, RGB end, float t)
{
    RGB result;
    float m = sqrtf(t) * 2.f - t;
    result.r = start.r + m * (end.r - start.r);
    result.g = start.g + m * (end.g - start.g);
    result.b = start.b + m * (end.b - start.b);

    return result;
}

HSV math::regress(HSV start, HSV end, float t)
{
    HSV result;
    float m = sqrtf(t) * 2.f - t;
    result.h = start.h + m * (end.h - start.h);
    result.s = start.s + m * (end.s - start.s);
    result.v = start.v + m * (end.v - start.v);

    return result;
}

glm::vec2 math::regress(const glm::vec2& start, const glm::vec2& end, float t)
{
    glm::vec2 result;
    float m = sqrtf(t) * 2.f - t;
    result.x = start.x + m * (end.x - start.x);
    result.y = start.y + m * (end.y - start.y);

    return result;
}

glm::mat3 math::rotateVectorToAnother(const glm::vec3& f, const glm::vec3& t)
{
    float c = glm::dot(f, t);

    if (glm::abs(c) > 0.999f)
    {
        glm::vec3 p;
        float absFX = glm::abs(f.x);
        float absFY = glm::abs(f.y);
        float absFZ = glm::abs(f.z);

        if (absFX < absFY && absFX < absFZ)
            p = glm::vec3(1.0f, 0.0f, 0.0f);
        else if (absFY < absFZ)
            p = glm::vec3(0.0f, 1.0f, 0.0f);
        else
            p = glm::vec3(0.0f, 0.0f, 1.0f);;

        glm::vec3 u = p - f;
        glm::vec3 v = p - t;

        float c0 = 2.0f / glm::dot(u, u);
        float c1 = 2.0f / glm::dot(v, v);
        float c2 = (4.f * glm::dot(u, v)) / (glm::dot(u, u) * glm::dot(v, v));

        return glm::mat3(1.0f - c0 * u.x * u.x - c1 * v.x * v.x + c2 * v.x * u.x,
                         -c0 * u.y * u.x - c1 * v.y * v.x + c2 * v.y * u.x,
                         -c0 * u.z * u.x - c1 * v.z * v.x + c2 * v.z * u.x,

                         -c0 * u.x * u.y - c1 * v.x * v.y + c2 * v.x * u.y,
                         1.0f - c0 * u.y * u.y - c1 * v.y * v.y + c2 * v.y * u.y,
                         -c0 * u.z * u.y - c1 * v.z * v.y + c2 * v.z * u.y,

                         -c0 * u.x * u.z - c1 * v.x * v.z + c2 * v.x * u.z,
                         -c0 * u.y * u.z - c1 * v.y * v.z + c2 * v.y * u.z,
                         1.0f - c0 * u.z * u.z - c1 * v.z * v.z + c2 * v.z * u.z);
    }

    auto v = glm::cross(t, f);
    float h = (1.0f - c) / glm::dot(v, v);

    return glm::mat3(c + h * v.x * v.x, h * v.x * v.y - v.z, h * v.x * v.z + v.y,
                     h * v.x * v.y + v.z, c + h * v.y * v.y, h * v.y * v.z - v.x,
                     h * v.x * v.z - v.y, h * v.y * v.z + v.x, c + h * v.z * v.z);
}

glm::mat4 math::rotationX(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return glm::mat4(1.f, 0.f, 0.f, 0.f,
                     0.f, c, s, 0.f,
                     0.f, -s, c, 0.f,
                     0.f, 0.f, 0.f, 1.f);
}

glm::mat4 math::rotationY(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return glm::mat4(c, 0.f, -s, 0.f,
                     0.f, 1.f, 0.f, 0.f,
                     s, 0.f, c, 0.f,
                     0.f, 0.f, 0.f, 1.f);
}

glm::mat4 math::rotationZ(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return glm::mat4(c, s, 0.f, 0.f,
                     -s, c, 0.f, 0.f,
                     0.f, 0.f, 1.f, 0.f,
                     0.f, 0.f, 0.f, 1.f);
}

glm::quat math::eulerYXZQuat(const glm::vec3& eulerAngles) { return eulerYXZQuat(eulerAngles.y, eulerAngles.x, eulerAngles.z); }

glm::quat math::eulerYXZQuat(float yaw, float pitch, float roll)
{
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);

    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);

    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);

    return glm::quat(cy * cp * cr + sy * sp * sr,
                     cy * sp * cr + sy * cp * sr,
                     sy * cp * cr - cy * sp * sr,
                     cy * cp * sr - sy * sp * cr);
}

glm::quat math::yawPitchRoll(float yaw, float pitch, float roll) { return eulerYXZQuat(yaw, pitch, roll); }
