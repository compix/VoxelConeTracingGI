#pragma once
#include <glm/glm.hpp>
#include <stdint.h>

class Rect
{
public:
    Rect() noexcept;
    Rect(const glm::vec2& p1, const glm::vec2& p2) noexcept;
    Rect(float minX, float minY, float maxX, float maxY) noexcept;

    void unite(const glm::vec2& p) noexcept;
    void unite(const Rect& b) noexcept;

    bool overlaps(const Rect& b) const noexcept;
    bool inside(const glm::vec2& p) const noexcept;

    void expand(float delta) noexcept;
    float area() const noexcept;

    glm::vec2 center() const noexcept { return m_min * 0.5f + m_max * 0.5f; }

    /**
    * Returns width (x), height (y).
    */
    glm::vec2 scale() const noexcept { return m_max - m_min; }

    /**
    * Returns the transformed Rect.
    */
    Rect toWorld(const glm::mat4& world) const noexcept;

    const glm::vec2& min() const noexcept { return m_min; }

    const glm::vec2& max() const noexcept { return m_max; }

    glm::vec2& min() noexcept { return m_min; }

    glm::vec2& max() noexcept { return m_max; }

    float width() const noexcept { return m_max.x - m_min.x; }

    float height() const noexcept { return m_max.y - m_min.y; }

    float minX() const noexcept { return m_min.x; }

    float minY() const noexcept { return m_min.y; }

private:
    glm::vec2 m_min, m_max;
};
