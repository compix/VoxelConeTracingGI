#include "Ray.h"
#include <engine/rendering/geometry/Mesh.h>
#include "Transform.h"

/**
* Adaptation of the "Slabs Method" from Real-Time Rendering 3rd Edition
*/
bool Ray::intersects(const OBBox& obb, float& t) const
{
    float tMin = -FLT_MAX;
    float tMax = FLT_MAX;

    glm::vec3 obbCenter(obb.model[3]);
    glm::vec3 p = obbCenter - origin;

    // Check intersections with all axes
    for (uint8_t i = 0; i < 3; ++i)
    {
        glm::vec3 axis = glm::normalize(glm::vec3(obb.model[i]));

        float e = glm::dot(axis, p);
        float f = glm::dot(axis, direction); // = cos(angle between axis and direction) since axis and direction are unit vectors

        if (std::abs(f) > math::EPSILON)
        {
            // Compute intersection points. This is just simple trigonometry
            float t1 = (e + obb.he[i]) / f;
            float t2 = (e - obb.he[i]) / f;

            if (t1 > t2)
                std::swap(t1, t2);

            if (t1 > tMin)
                tMin = t1;

            if (t2 < tMax)
                tMax = t2;

            // If the ray misses or the obb is "behind" the ray then there is no intersection
            if (tMin > tMax || tMax < 0)
                return false;
        }
        // Axis and ray are parallel -> check if ray is outside the slab
        else if (-e - obb.he[i] > 0 || -e + obb.he[i] < 0)
            return false;
    }

    if (tMin > 0)
    {
        t = tMin;
        return true;
    }

    t = tMax;
    return true;
}

bool Ray::intersectsTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, glm::vec2& uv, float& t) const
{
    glm::vec3 e1 = p1 - p0;
    glm::vec3 e2 = p2 - p0;

    glm::vec3 q = glm::cross(direction, e2);

    float a = glm::dot(e1, q);

    if (a > -math::EPSILON5 && a < math::EPSILON5)
        return false;

    float f = 1.f / a;

    glm::vec3 s = origin - p0;
    uv.x = f * glm::dot(s, q);

    if (uv.x < 0.f)
        return false;

    glm::vec3 r = glm::cross(s, e1);

    uv.y = f * glm::dot(direction, r);

    if (uv.y < 0.f || (uv.x + uv.y) > 1.f)
        return false;

    t = f * glm::dot(e2, r);

    return true;
}

/*
bool Ray::intersects(const Mesh& mesh, const Transform& transform, glm::vec3& normal, float& t) const
{
    const MeshData& md = mesh.getMeshData();
    glm::vec2 uv;
    float minT = std::numeric_limits<float>().max();
    bool hit = false;
    GLuint minIdx = 0;

    // Go through all triangles and do an intersection test
    for (GLuint i = 0; i < md.indices.size() / 3; ++i)
    {
        glm::vec3 p0 = transform.transformPointToWorld(md.vertices[md.indices[3 * i]].pos);
        glm::vec3 p1 = transform.transformPointToWorld(md.vertices[md.indices[3 * i + 1]].pos);
        glm::vec3 p2 = transform.transformPointToWorld(md.vertices[md.indices[3 * i + 2]].pos);

        if (intersectsTriangle(p0, p1, p2, uv, t))
        {
            if (t > 0.0f && t < minT)
            {
                minT = t;
                minIdx = i;
            }
            
            hit = true;
        }
    }

    if (hit)
    {
        glm::vec3 p0 = transform.transformPointToWorld(md.vertices[md.indices[3 * minIdx]].pos);
        glm::vec3 p1 = transform.transformPointToWorld(md.vertices[md.indices[3 * minIdx + 1]].pos);
        glm::vec3 p2 = transform.transformPointToWorld(md.vertices[md.indices[3 * minIdx + 2]].pos);
        normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
        t = minT;
    }
    
    return hit;

    return true;
}*/

bool Ray::intersectsSphere(const glm::vec3& pos, const float& radius, float& t) const
{
    glm::vec3 toSphere = origin - pos;
    float a = glm::dot(direction, direction);
    float b = glm::dot(direction, (2.0f * toSphere));
    float c = glm::dot(toSphere, toSphere) - radius * radius;

    float d = b * b - 4.0f * a * c;

    if (d > 0.0f)
    {
        t = (-b - sqrtf(d)) / (2.0f * a);

        if (t > 0.0f)
            return true;
    }

    return false;
}
