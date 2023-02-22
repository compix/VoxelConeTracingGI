#include "EntityCreator.h"
#include "engine/rendering/geometry/GeometryGenerator.h"
#include "engine/rendering/renderer/MeshRenderers.h"
#include "engine/ecs/ECS.h"
#include "engine/geometry/Transform.h"
#include "engine/rendering/lights/DirectionalLight.h"
#include "engine/util/math.h"
#include "engine/resource/ResourceManager.h"
#include <cstddef>

std::size_t EntityCreator::m_boxCounter = 0;
std::size_t EntityCreator::m_cylinderCounter = 0;
std::size_t EntityCreator::m_sphereCounter = 0;
std::size_t EntityCreator::m_dirLightCounter = 0;

Entity EntityCreator::createBox(const std::string& name, const glm::vec3& pos, const glm::vec3& scale)
{
    auto material = createMaterial();

    auto mesh = std::make_shared<Mesh>();
    mesh->load(GeometryGenerator::createBox(1.0f, 1.0f, 1.0f));
    auto meshRenderer = std::make_shared<MeshRenderer>(mesh);
    meshRenderer->addMaterial(material);

    Entity entity = ECS::createEntity(name);
    entity.addComponent<MeshRenderer>(*meshRenderer);
    entity.addComponent<Transform>();

    auto transform = entity.getComponent<Transform>();
    transform->setPosition(pos);
    transform->setLocalScale(scale);
    transform->setBBox(mesh->computeBBox());

    return entity;
}

Entity EntityCreator::createCylinder(const std::string& name, const glm::vec3& pos, const glm::vec3& scale)
{
    auto material = createMaterial();

    auto mesh = std::make_shared<Mesh>();
    mesh->load(GeometryGenerator::createCylinder());
    auto meshRenderer = std::make_shared<MeshRenderer>(mesh);
    meshRenderer->addMaterial(material);

    Entity entity = ECS::createEntity(name);
    entity.addComponent<MeshRenderer>(*meshRenderer);
    entity.addComponent<Transform>();

    auto transform = entity.getComponent<Transform>();
    transform->setPosition(pos);
    transform->setLocalScale(scale);
    transform->setBBox(mesh->computeBBox());

    return entity;
}

Entity EntityCreator::createSphere(const std::string& name, const glm::vec3& pos, const glm::vec3& scale)
{
    auto material = createMaterial();

    auto mesh = std::make_shared<Mesh>();
    mesh->load(GeometryGenerator::createSphere(0.5f, 32, 32));
    auto meshRenderer = std::make_shared<MeshRenderer>(mesh);
    meshRenderer->addMaterial(material);

    Entity entity = ECS::createEntity(name);
    entity.addComponent<MeshRenderer>(*meshRenderer);
    entity.addComponent<Transform>();

    auto transform = entity.getComponent<Transform>();
    transform->setPosition(pos);
    transform->setLocalScale(scale);
    transform->setBBox(mesh->computeBBox());

    return entity;
}

Entity EntityCreator::createDefaultBox(const glm::vec3& pos)
{
    auto name = "Box" + std::to_string(m_boxCounter);

    ++m_boxCounter;

    return createBox(name, pos, glm::vec3(1.0f));
}

Entity EntityCreator::createDefaultCylinder(const glm::vec3& pos)
{
    auto name = "Cylinder" + std::to_string(m_cylinderCounter);

    ++m_cylinderCounter;

    return createCylinder(name, pos, glm::vec3(1.0f));
}

Entity EntityCreator::createDefaultSphere(const glm::vec3& pos)
{
    auto name = "Sphere" + std::to_string(m_sphereCounter);

    ++m_sphereCounter;

    return createSphere(name, pos, glm::vec3(1.0f));
}

Entity EntityCreator::createDefaultDirLight()
{
    auto name = "DirLight" + std::to_string(m_dirLightCounter);

    ++m_dirLightCounter;

    Entity dirLight = ECS::createEntity(name);
    dirLight.addComponent<DirectionalLight>();
    dirLight.addComponent<Transform>();
    dirLight.getComponent<Transform>()->setPosition(glm::vec3(0.0f, 20.0f, 0.f));
    dirLight.getComponent<Transform>()->setEulerAngles(glm::vec3(math::toRadians(60.0f), 0.0f, 0.0f));

    return dirLight;
}

std::shared_ptr<Material> EntityCreator::createMaterial()
{
    auto material = std::make_shared<Material>(ResourceManager::getShader("shaders/forwardShadingPass.vert", "shaders/forwardShadingPass.frag"));
    material->setFloat("u_hasDiffuseTexture", 0.0f);
    material->setFloat("u_hasNormalMap", 0.0f);
    material->setFloat("u_hasSpecularMap", 0.0f);
    material->setFloat("u_hasEmissionMap", 0.0f);
    material->setFloat("u_hasOpacityMap", 0.0f);
    material->setFloat("u_shininess", 40.0f);
    material->setColor("u_color", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    material->setColor("u_emissionColor", glm::vec3(0.0f));
    material->setColor("u_specularColor", glm::vec3(0.0f));

    return material;
}
