#pragma once
#include <glm/glm.hpp>
#include <stdint.h>
#include <initializer_list>

class BBox
{
    friend class SVOBuilder;

public:
    BBox() noexcept;
    BBox(const glm::vec3& min, const glm::vec3& max) noexcept;

    void unite(const std::initializer_list<glm::vec3>& ps);
    void unite(const glm::vec3& p) noexcept;
    void unite(const BBox& b) noexcept;

    bool overlaps(const BBox& b) const noexcept;
    bool inside(const glm::vec3& p) const noexcept;

    void expand(float delta) noexcept;
    float surfaceArea() const noexcept;
    float volume() const noexcept;

    // Returns the number of the longest axis
    // 0 for x, 1 for y and 2 for z
    uint8_t maxExtentIdx() const noexcept;

    // Returns the number of the shortest axis
    // 0 for x, 1 for y and 2 for z
    uint8_t minExtentIdx() const noexcept;

    float maxExtent() const noexcept;
    float minExtent() const noexcept;

    glm::vec3 center() const noexcept { return m_min * 0.5f + m_max * 0.5f; }

    // Returns width(x), height(y), depth(z)
    glm::vec3 scale() const noexcept { return m_max - m_min; }

    // Returns the transformed BBox.
    BBox toWorld(const glm::mat4& world) const noexcept;

    void transform(const glm::mat4& m) noexcept;

    glm::mat4 world() const;

    const auto& min() const noexcept { return m_min; }

    const auto& max() const noexcept { return m_max; }

private:
    glm::vec3 m_min, m_max;
};
