#pragma once
#include <glm/glm.hpp>
#include "OBBox.h"

class Transform;
class Mesh;

struct Ray
{
    Ray() { }

    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(direction) { }

    bool intersects(const OBBox& obb, float& t) const;

    /**
    * Performs an intersection test between this ray and a given triangle defined by
    * points (p0, p1, p2). If there is no intersection false is returned and (uv, t) will be undefined
    * otherwise true is returned and (uv, t) set to the correct values.
    */
    bool intersectsTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, glm::vec2& uv, float& t) const;

    bool intersects(const Mesh& mesh, const Transform& transform, glm::vec3& normal, float& t) const = delete;

    bool intersectsSphere(const glm::vec3& pos, const float& radius, float& t) const;

    glm::vec3 origin;
    glm::vec3 direction;
};
