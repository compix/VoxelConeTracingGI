#pragma once
#include <glm/ext.hpp>

class Rect;
class CameraComponent;

/**
* Simple arcball model rotation based on: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
* Call onRotationUpdate() when dragging occurs in the correct section of the screen to update the rotation otherwise call onIdle().
*/
class ArcballRotator
{
public:
    ArcballRotator(float speed = 1.5f)
        : m_speed(speed) { }

    void onRotationUpdate(const Rect& viewport, const glm::mat4& viewInverse, const glm::vec2& startDragPos, const glm::vec2& curDragPos);
    void onIdle();

    const glm::quat& getRotation() const { return m_rotation; }

    void setRotation(const glm::quat& rotation)
    {
        m_rotation = rotation;
        m_rotationBeforeDrag = m_rotation;
    }

    const glm::vec3& getArcballVector() const { return m_curArcballVector; }

private:
    glm::vec3 computeArcballVector(glm::vec2 pos) const;

    glm::vec3 m_curArcballVector;
    glm::quat m_rotation;
    glm::quat m_rotationBeforeDrag;
    float m_speed;
};
