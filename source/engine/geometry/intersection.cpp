#include "intersection.h"
#include <algorithm>
#include "BBox.h"

bool intersection::bboxTriangle(const BBox& b, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    // BBox/BBox test
    BBox triangleBBox;
    triangleBBox.unite({v0, v1, v2});

    if (!b.overlaps(triangleBBox))
        return false;

    // Triangle plane/BBox test
    glm::vec3 e0 = v1 - v0;
    glm::vec3 e1 = v2 - v1;
    glm::vec3 e2 = v0 - v2;

    glm::vec3 n = glm::normalize(glm::cross(e0, e1));
    glm::vec3 dp = b.scale();
    glm::vec3 c = glm::vec3(
        n.x > 0.0f ? dp.x : 0.0f,
        n.y > 0.0f ? dp.y : 0.0f,
        n.z > 0.0f ? dp.z : 0.0f);

    float d1 = glm::dot(n, c - v0);
    float d2 = glm::dot(n, (dp - c) - v0);

    if ((glm::dot(n, b.min()) + d1) * (glm::dot(n, b.min()) + d2) > 0.0f)
        return false;

    // 2D Projections of Triangle/BBox test

    // XY Plane
    glm::vec2 n_xy_e0(-e0.y, e0.x);
    glm::vec2 n_xy_e1(-e1.y, e1.x);
    glm::vec2 n_xy_e2(-e2.y, e2.x);

    if (n.z < 0.0f)
    {
        n_xy_e0 *= -1.0f;
        n_xy_e1 *= -1.0f;
        n_xy_e2 *= -1.0f;
    }

    float d_xy_e0 = -glm::dot(n_xy_e0, glm::vec2(v0.x, v0.y)) + std::max(0.0f, dp.x * n_xy_e0[0]) + std::max(0.0f, dp.y * n_xy_e0[1]);
    float d_xy_e1 = -glm::dot(n_xy_e1, glm::vec2(v1.x, v1.y)) + std::max(0.0f, dp.x * n_xy_e1[0]) + std::max(0.0f, dp.y * n_xy_e1[1]);
    float d_xy_e2 = -glm::dot(n_xy_e2, glm::vec2(v2.x, v2.y)) + std::max(0.0f, dp.x * n_xy_e2[0]) + std::max(0.0f, dp.y * n_xy_e2[1]);

    glm::vec2 p_xy(b.min().x, b.min().y);

    if ((glm::dot(n_xy_e0, p_xy) + d_xy_e0 < 0.0f) ||
        (glm::dot(n_xy_e1, p_xy) + d_xy_e1 < 0.0f) ||
        (glm::dot(n_xy_e2, p_xy) + d_xy_e2 < 0.0f))
        return false;

    // ZX Plane
    glm::vec2 n_zx_e0(-e0.x, e0.z);
    glm::vec2 n_zx_e1(-e1.x, e1.z);
    glm::vec2 n_zx_e2(-e2.x, e2.z);

    if (n.y < 0.0f)
    {
        n_zx_e0 *= -1.0f;
        n_zx_e1 *= -1.0f;
        n_zx_e2 *= -1.0f;
    }

    float d_zx_e0 = -glm::dot(n_zx_e0, glm::vec2(v0.z, v0.x)) + std::max(0.0f, dp.z * n_zx_e0[0]) + std::max(0.0f, dp.x * n_zx_e0[1]);
    float d_zx_e1 = -glm::dot(n_zx_e1, glm::vec2(v1.z, v1.x)) + std::max(0.0f, dp.z * n_zx_e1[0]) + std::max(0.0f, dp.x * n_zx_e1[1]);
    float d_zx_e2 = -glm::dot(n_zx_e2, glm::vec2(v2.z, v2.x)) + std::max(0.0f, dp.z * n_zx_e2[0]) + std::max(0.0f, dp.x * n_zx_e2[1]);

    glm::vec2 p_zx(b.min().z, b.min().x);

    if ((glm::dot(n_zx_e0, p_zx) + d_zx_e0 < 0.0f) ||
        (glm::dot(n_zx_e1, p_zx) + d_zx_e1 < 0.0f) ||
        (glm::dot(n_zx_e2, p_zx) + d_zx_e2 < 0.0f))
        return false;

    // YZ Plane
    glm::vec2 n_yz_e0(-e0.z, e0.y);
    glm::vec2 n_yz_e1(-e1.z, e1.y);
    glm::vec2 n_yz_e2(-e2.z, e2.y);

    if (n.x < 0.0f)
    {
        n_yz_e0 *= -1.0f;
        n_yz_e1 *= -1.0f;
        n_yz_e2 *= -1.0f;
    }

    float d_yz_e0 = -glm::dot(n_yz_e0, glm::vec2(v0.y, v0.z)) + std::max(0.0f, dp.y * n_yz_e0[0]) + std::max(0.0f, dp.z * n_yz_e0[1]);
    float d_yz_e1 = -glm::dot(n_yz_e1, glm::vec2(v1.y, v1.z)) + std::max(0.0f, dp.y * n_yz_e1[0]) + std::max(0.0f, dp.z * n_yz_e1[1]);
    float d_yz_e2 = -glm::dot(n_yz_e2, glm::vec2(v2.y, v2.z)) + std::max(0.0f, dp.y * n_yz_e2[0]) + std::max(0.0f, dp.z * n_yz_e2[1]);

    glm::vec2 p_yz(b.min().y, b.min().z);

    if ((glm::dot(n_yz_e0, p_yz) + d_yz_e0 < 0.0f) ||
        (glm::dot(n_yz_e1, p_yz) + d_yz_e1 < 0.0f) ||
        (glm::dot(n_yz_e2, p_yz) + d_yz_e2 < 0.0f))
        return false;

    return true;
}

void intersection::TriangleBBox::setTriangle(const glm::vec3& pv0, const glm::vec3& pv1, const glm::vec3& pv2)
{
    this->v0 = pv0;
    this->v1 = pv1;
    this->v2 = pv2;

    // BBox/BBox test
    triangleBBox = BBox();
    triangleBBox.unite({v0, v1, v2});

    // Triangle plane/BBox test
    glm::vec3 e0 = v1 - v0;
    glm::vec3 e1 = v2 - v1;
    glm::vec3 e2 = v0 - v2;

    n = glm::normalize(glm::cross(e0, e1));

    // 2D Projections of Triangle/BBox test

    // XY Plane
    n_xy_e0 = glm::vec2(-e0.y, e0.x);
    n_xy_e1 = glm::vec2(-e1.y, e1.x);
    n_xy_e2 = glm::vec2(-e2.y, e2.x);

    if (n.z < 0.0f)
    {
        n_xy_e0 *= -1.0f;
        n_xy_e1 *= -1.0f;
        n_xy_e2 *= -1.0f;
    }

    // ZX Plane
    n_zx_e0 = glm::vec2(-e0.x, e0.z);
    n_zx_e1 = glm::vec2(-e1.x, e1.z);
    n_zx_e2 = glm::vec2(-e2.x, e2.z);

    if (n.y < 0.0f)
    {
        n_zx_e0 *= -1.0f;
        n_zx_e1 *= -1.0f;
        n_zx_e2 *= -1.0f;
    }

    // YZ Plane
    n_yz_e0 = glm::vec2(-e0.z, e0.y);
    n_yz_e1 = glm::vec2(-e1.z, e1.y);
    n_yz_e2 = glm::vec2(-e2.z, e2.y);

    if (n.x < 0.0f)
    {
        n_yz_e0 *= -1.0f;
        n_yz_e1 *= -1.0f;
        n_yz_e2 *= -1.0f;
    }
}

void intersection::TriangleBBox::setBBoxScale(const glm::vec3& scale)
{
    dp = scale;
    glm::vec3 c = glm::vec3(
        n.x > 0.0f ? dp.x : 0.0f,
        n.y > 0.0f ? dp.y : 0.0f,
        n.z > 0.0f ? dp.z : 0.0f);

    d1 = glm::dot(n, c - v0);
    d2 = glm::dot(n, (dp - c) - v0);

    d_xy_e0 = -glm::dot(n_xy_e0, glm::vec2(v0.x, v0.y)) + std::max(0.0f, dp.x * n_xy_e0[0]) + std::max(0.0f, dp.y * n_xy_e0[1]);
    d_xy_e1 = -glm::dot(n_xy_e1, glm::vec2(v1.x, v1.y)) + std::max(0.0f, dp.x * n_xy_e1[0]) + std::max(0.0f, dp.y * n_xy_e1[1]);
    d_xy_e2 = -glm::dot(n_xy_e2, glm::vec2(v2.x, v2.y)) + std::max(0.0f, dp.x * n_xy_e2[0]) + std::max(0.0f, dp.y * n_xy_e2[1]);

    d_zx_e0 = -glm::dot(n_zx_e0, glm::vec2(v0.z, v0.x)) + std::max(0.0f, dp.z * n_zx_e0[0]) + std::max(0.0f, dp.x * n_zx_e0[1]);
    d_zx_e1 = -glm::dot(n_zx_e1, glm::vec2(v1.z, v1.x)) + std::max(0.0f, dp.z * n_zx_e1[0]) + std::max(0.0f, dp.x * n_zx_e1[1]);
    d_zx_e2 = -glm::dot(n_zx_e2, glm::vec2(v2.z, v2.x)) + std::max(0.0f, dp.z * n_zx_e2[0]) + std::max(0.0f, dp.x * n_zx_e2[1]);

    d_yz_e0 = -glm::dot(n_yz_e0, glm::vec2(v0.y, v0.z)) + std::max(0.0f, dp.y * n_yz_e0[0]) + std::max(0.0f, dp.z * n_yz_e0[1]);
    d_yz_e1 = -glm::dot(n_yz_e1, glm::vec2(v1.y, v1.z)) + std::max(0.0f, dp.y * n_yz_e1[0]) + std::max(0.0f, dp.z * n_yz_e1[1]);
    d_yz_e2 = -glm::dot(n_yz_e2, glm::vec2(v2.y, v2.z)) + std::max(0.0f, dp.y * n_yz_e2[0]) + std::max(0.0f, dp.z * n_yz_e2[1]);
}

bool intersection::TriangleBBox::test(const BBox& b)
{
    if (!b.overlaps(triangleBBox))
        return false;

    if ((glm::dot(n, b.min()) + d1) * (glm::dot(n, b.min()) + d2) > 0.0f)
        return false;

    glm::vec2 p_xy(b.min().x, b.min().y);

    if ((glm::dot(n_xy_e0, p_xy) + d_xy_e0 < 0.0f) ||
        (glm::dot(n_xy_e1, p_xy) + d_xy_e1 < 0.0f) ||
        (glm::dot(n_xy_e2, p_xy) + d_xy_e2 < 0.0f))
        return false;

    glm::vec2 p_zx(b.min().z, b.min().x);

    if ((glm::dot(n_zx_e0, p_zx) + d_zx_e0 < 0.0f) ||
        (glm::dot(n_zx_e1, p_zx) + d_zx_e1 < 0.0f) ||
        (glm::dot(n_zx_e2, p_zx) + d_zx_e2 < 0.0f))
        return false;

    glm::vec2 p_yz(b.min().y, b.min().z);

    if ((glm::dot(n_yz_e0, p_yz) + d_yz_e0 < 0.0f) ||
        (glm::dot(n_yz_e1, p_yz) + d_yz_e1 < 0.0f) ||
        (glm::dot(n_yz_e2, p_yz) + d_yz_e2 < 0.0f))
        return false;

    return true;
}
