#include "ArcballRotator.h"
#include <algorithm>
#include <cmath>
#include "engine/geometry/Rect.h"

void ArcballRotator::onRotationUpdate(const Rect& viewport, const glm::mat4& viewInverse, const glm::vec2& startDragPos, const glm::vec2& curDragPos)
{
    if (glm::length2(curDragPos - startDragPos) == 0.0f)
        return;

    const Rect& vp = viewport;
    float viewportRadius = 0.5f * sqrtf(vp.width() * vp.width() + vp.height() * vp.height());
    glm::vec2 viewportCenter = vp.center();

    glm::vec2 startPos = (startDragPos - viewportCenter) / viewportRadius;
    glm::vec2 curPos = (curDragPos - viewportCenter) / viewportRadius;

    glm::vec3 toStart = glm::vec3(viewInverse * glm::vec4(computeArcballVector(startPos), 0.0f));
    m_curArcballVector = computeArcballVector(curPos);
    glm::vec3 toEnd = glm::vec3(viewInverse * glm::vec4(m_curArcballVector, 0.0f));

    float angle = m_speed * acosf(std::min(1.0f, glm::dot(toStart, toEnd)));
    glm::vec3 rotationAxis = glm::normalize(glm::cross(toStart, toEnd));
    m_rotation = glm::angleAxis(angle, rotationAxis) * m_rotationBeforeDrag;
}

void ArcballRotator::onIdle()
{
    m_rotationBeforeDrag = m_rotation;
}

glm::vec3 ArcballRotator::computeArcballVector(glm::vec2 pos) const
{
    // Radius of the ball
    static float r = 1.0f;

    // Use pythagorean theorem to compute z
    glm::vec3 v = glm::vec3(pos, 0.0f);
    float sq = v.x * v.x + v.y * v.y;
    if (sq <= r * r)
        v.z = -sqrtf(r * r - sq);

    return glm::normalize(v);
}
