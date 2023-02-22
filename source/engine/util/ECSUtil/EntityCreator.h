#pragma once
#include "engine/ecs/EntityManager.h"
#include <glm/glm.hpp>
#include "engine/rendering/Material.h"
#include <cstddef>

class EntityCreator
{
public:
    static Entity createBox(const std::string& name, const glm::vec3& pos, const glm::vec3& scale);
    static Entity createCylinder(const std::string& name, const glm::vec3& pos, const glm::vec3& scale);
    static Entity createSphere(const std::string& name, const glm::vec3& pos, const glm::vec3& scale);

    static Entity createDefaultBox(const glm::vec3& pos = glm::vec3(0.0f));
    static Entity createDefaultCylinder(const glm::vec3& pos = glm::vec3(0.0f));
    static Entity createDefaultSphere(const glm::vec3& pos = glm::vec3(0.0f));
    static Entity createDefaultDirLight();

private:
    static std::shared_ptr<Material> createMaterial();

private:
    static std::size_t m_boxCounter;
    static std::size_t m_cylinderCounter;
    static std::size_t m_sphereCounter;
    static std::size_t m_dirLightCounter;
};
