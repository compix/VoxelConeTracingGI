#include "Rect.h"
#include <algorithm>

Rect::Rect() noexcept
    : m_min(FLT_MAX, FLT_MAX),
      m_max(-FLT_MAX, -FLT_MAX) {}

Rect::Rect(const glm::vec2& p1, const glm::vec2& p2) noexcept
    : m_min(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
      m_max(std::max(p1.x, p2.x), std::max(p1.y, p2.y)) {}

Rect::Rect(float minX, float minY, float maxX, float maxY) noexcept
    : m_min(minX, minY), m_max(maxX, maxY) {}

void Rect::unite(const glm::vec2& p) noexcept
{
    m_min.x = std::min(m_min.x, p.x);
    m_min.y = std::min(m_min.y, p.y);

    m_max.x = std::max(m_max.x, p.x);
    m_max.y = std::max(m_max.y, p.y);
}

void Rect::unite(const Rect& b) noexcept
{
    m_min.x = std::min(m_min.x, b.m_min.x);
    m_min.y = std::min(m_min.y, b.m_min.y);

    m_max.x = std::max(m_max.x, b.m_max.x);
    m_max.y = std::max(m_max.y, b.m_max.y);
}

bool Rect::overlaps(const Rect& b) const noexcept
{
    return ((m_max.x >= b.m_min.x) && (m_min.x <= b.m_max.x) &&
        (m_max.y >= b.m_min.y) && (m_min.y <= b.m_max.y));
}

bool Rect::inside(const glm::vec2& p) const noexcept
{
    return (p.x >= m_min.x && p.x <= m_max.x &&
        p.y >= m_min.y && p.y <= m_max.y);
}

void Rect::expand(float delta) noexcept
{
    m_min -= glm::vec2(delta, delta);
    m_max += glm::vec2(delta, delta);
}

float Rect::area() const noexcept
{
    auto d = m_max - m_min;
    return d.x * d.y;
}

Rect Rect::toWorld(const glm::mat4& world) const noexcept { return Rect(glm::vec2(world * glm::vec4(m_min, 0.f, 1.f)), glm::vec2(world * glm::vec4(m_max, 0.f, 1.f))); }
