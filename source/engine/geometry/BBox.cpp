#include "BBox.h"
#include <algorithm>
#include <glm/ext.hpp>

BBox::BBox() noexcept
    : m_min(FLT_MAX, FLT_MAX, FLT_MAX),
      m_max(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}

BBox::BBox(const glm::vec3& min, const glm::vec3& max) noexcept
    : m_min(min), m_max(max) {}

void BBox::unite(const std::initializer_list<glm::vec3>& ps)
{
    for (auto& p : ps)
        unite(p);
}

void BBox::unite(const glm::vec3& p) noexcept
{
    m_min = glm::min(m_min, p);
    m_max = glm::max(m_max, p);
}

void BBox::unite(const BBox& b) noexcept
{
    m_min = glm::min(m_min, b.m_min);
    m_max = glm::max(m_max, b.m_max);
}

bool BBox::overlaps(const BBox& b) const noexcept
{
    return ((m_max.x >= b.m_min.x) && (m_min.x <= b.m_max.x) &&
            (m_max.y >= b.m_min.y) && (m_min.y <= b.m_max.y) &&
            (m_max.z >= b.m_min.z) && (m_min.z <= b.m_max.z));
}

bool BBox::inside(const glm::vec3& p) const noexcept
{
    return (p.x >= m_min.x && p.x <= m_max.x &&
            p.y >= m_min.y && p.y <= m_max.y &&
            p.z >= m_min.z && p.z <= m_max.z);
}

void BBox::expand(float delta) noexcept
{
    m_min -= glm::vec3(delta, delta, delta);
    m_max += glm::vec3(delta, delta, delta);
}

float BBox::surfaceArea() const noexcept
{
    auto d = m_max - m_min;
    return 2.f * (d.x * d.y + d.x * d.z + d.y * d.z);
}

float BBox::volume() const noexcept
{
    auto d = m_max - m_min;
    return d.x * d.y * d.z;
}

uint8_t BBox::maxExtentIdx() const noexcept
{
    auto d = m_max - m_min;
    if (d.x > d.y && d.x > d.z)
        return 0;

    if (d.y > d.z)
        return 1;

    return 2;
}

uint8_t BBox::minExtentIdx() const noexcept
{
    auto d = m_max - m_min;
    if (d.x < d.y && d.x < d.z)
        return 0;

    if (d.y < d.z)
        return 1;

    return 2;
}

float BBox::maxExtent() const noexcept { return scale()[maxExtentIdx()]; }

float BBox::minExtent() const noexcept { return scale()[minExtentIdx()]; }

BBox BBox::toWorld(const glm::mat4& m) const noexcept
{
    glm::vec3 xa = glm::vec3(m[0] * m_min.x);
    glm::vec3 xb = glm::vec3(m[0] * m_max.x);

    glm::vec3 ya = glm::vec3(m[1] * m_min.y);
    glm::vec3 yb = glm::vec3(m[1] * m_max.y);

    glm::vec3 za = glm::vec3(m[2] * m_min.z);
    glm::vec3 zb = glm::vec3(m[2] * m_max.z);

    return BBox(glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + glm::vec3(m[3]),
                glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + glm::vec3(m[3]));
}

void BBox::transform(const glm::mat4& m) noexcept { *this = toWorld(m); }

glm::mat4 BBox::world() const { return glm::translate(center()) * glm::scale(scale()); }
