#pragma once
#include <engine/ecs/EntityManager.h>
#include <engine/rendering/shader/Shader.h>
#include <memory>
#include <engine/resource/Model.h>
#include "engine/ecs/ECS.h"

class ECSUtil
{
public:
    static ComponentPtr<Transform> loadMeshEntities(Model* model, std::shared_ptr<Shader> shader, const std::string& baseTexturePath, const glm::vec3& scale = glm::vec3(1.0f),
                                                    bool useDerivedPos = false, ComponentPtr<Transform> parent = ComponentPtr<Transform>());
    static ComponentPtr<Transform> loadMeshEntities(const std::string& path, std::shared_ptr<Shader> shader,
                                                    const std::string& baseTexturePath, const glm::vec3& scale = glm::vec3(1.0f), bool useDerivedPos = false);

    static void renderEntities(Shader* shader);

    static void renderEntities(const std::vector<Entity>& entities, Shader* shader);

    static void renderEntity(Entity entity, Shader* shader);

    static void renderEntitiesInAABB(const BBox& bbox, Shader* shader);

    template<class... Components>
    static std::vector<Entity> getEntitiesInAABB(const BBox& bbox);

    static void setDirectionalLightUniforms(Shader* shader, GLint shadowMapStartTextureUnit, float pcfRadius = -1.0f);
};

template <class ... Components>
std::vector<Entity> ECSUtil::getEntitiesInAABB(const BBox& bbox)
{
    std::vector<Entity> entities;

    for (Entity e : ECS::getEntitiesWithComponents<Components...>())
    {
        auto transform = e.getComponent<Transform>();
        assert(transform);

        if (bbox.overlaps(transform->getBBox()))
            entities.push_back(e);
    }

    return entities;
}
