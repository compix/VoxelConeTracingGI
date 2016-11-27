#pragma once
#include <glm/detail/type_vec3.hpp>
#include <engine/ecs/System.h>
#include <engine/geometry/Transform.h>
#include "engine/util/math.h"

class SpringCamera : public Component
{
    friend class SpringCameraSystem;
public:
    virtual ~SpringCamera() { }

    ComponentPtr<Transform> target;
protected:
    float m_elevation = math::PI / 4.0f;
    float m_azimuth = 0.0f;

    glm::vec3 m_targetOffset{0, 0, 0};

    float m_distance = 3.0f;
    float m_actualDistance = 3.0f;

    float m_distSpringConstant = 150.0f;
    float m_rotationSpeed = 0.05f;

    glm::vec3 m_curVelocity;
    glm::vec3 m_lookVelocity;
    glm::vec3 m_curLookAtPos;

    glm::vec3 m_cameraDestination;
    glm::vec3 m_targetPos;
};

class SpringFreeLookCamera : public SpringCamera
{
    friend class SpringCameraSystem;

    float m_minElevation = 45.0f;
    float m_maxElevation = 75.0f;
    float m_lookSpeedJoypad = 60.0f;
    float m_lookSpeedMouse = 0.5f;

public:
    float horizontal;
    float vertical;
};

class SpringCameraSystem : public System
{
public:
    void update(EntityManager& entityManager) override;

private:
    void updatePosition(ComponentPtr<SpringFreeLookCamera> camera, ComponentPtr<Transform> transform);
    void updateLookAt(ComponentPtr<SpringFreeLookCamera> camera, ComponentPtr<Transform> transform);
    glm::vec3 computeSpringAccel(ComponentPtr<SpringFreeLookCamera> camera,
                                 const glm::vec3& pos, const glm::vec3& target, const glm::vec3& velocity);
};
