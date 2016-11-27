#include "SpringCamera.h"
#include <engine/ecs/EntityManager.h>
#include <engine/geometry/Transform.h>
#include <engine/util/Timer.h>

void SpringCameraSystem::update(EntityManager& entityManager)
{
    for (Entity entity : entityManager.getEntitiesWithComponents<Transform, SpringFreeLookCamera>())
    {
        auto transform = entity.getComponent<Transform>();
        auto camera = entity.getComponent<SpringFreeLookCamera>();

        float x = camera->horizontal;
        float y = camera->vertical;

        camera->m_elevation = glm::clamp(camera->m_elevation + y * camera->m_rotationSpeed,
                                         -math::TO_RAD * camera->m_minElevation, math::TO_RAD * camera->m_maxElevation);

        camera->m_azimuth += x * camera->m_rotationSpeed;

        glm::vec3 polar = math::polarToVector(camera->m_actualDistance, camera->m_elevation, camera->m_azimuth);

        camera->m_targetPos = camera->target->getPosition() + camera->m_targetOffset;
        camera->m_cameraDestination = camera->target->getPosition() + polar + camera->m_targetOffset;

        updatePosition(camera, transform);
        updateLookAt(camera, transform);
    }
}

void SpringCameraSystem::updateLookAt(ComponentPtr<SpringFreeLookCamera> camera, ComponentPtr<Transform> transform)
{
    glm::vec3 lookAccel = computeSpringAccel(camera, camera->m_curLookAtPos, camera->m_targetPos, camera->m_lookVelocity);
    camera->m_lookVelocity += lookAccel * Time::deltaTime();
    camera->m_curLookAtPos += camera->m_lookVelocity * Time::deltaTime();
    transform->lookAt(camera->m_curLookAtPos);
}

void SpringCameraSystem::updatePosition(ComponentPtr<SpringFreeLookCamera> camera, ComponentPtr<Transform> transform)
{
    glm::vec3 accel = computeSpringAccel(camera, transform->getPosition(), camera->m_cameraDestination, camera->m_curVelocity);
    camera->m_curVelocity += accel * Time::deltaTime();
    transform->setPosition(transform->getPosition() + camera->m_curVelocity * Time::deltaTime());
}

// Computes the acceleration using a critical damping spring model
glm::vec3 SpringCameraSystem::computeSpringAccel(ComponentPtr<SpringFreeLookCamera> camera,
                                                 const glm::vec3& pos, const glm::vec3& target, const glm::vec3& velocity)
{
    float dampingCoefficient = 2.0f * sqrtf(camera->m_distSpringConstant);

    glm::vec3 deltaAccel = (target - pos) * camera->m_distSpringConstant;
    glm::vec3 dampingAccel = -velocity * dampingCoefficient;

    return deltaAccel + dampingAccel;
}
