#include "ECS.h"

EntityManager ECS::m_entityManager;
std::unordered_map<std::type_index, System*> ECS::m_systems;
std::unordered_map<std::string, Entity> ECS::m_entityMap;

void ECS::update()
{
    for (auto& sp : m_systems)
        sp.second->update(m_entityManager);

    m_entityManager.update();
}

void ECS::lateUpdate()
{
    for (auto& sp : m_systems)
        sp.second->lateUpdate(m_entityManager);

    m_entityManager.lateUpdate();
}

Entity ECS::createEntity(const std::string& name)
{
    Entity entity = m_entityManager.create(name);
    m_entityMap[name] = entity;
    return entity;
}

Entity ECS::createEntity()
{
    Entity entity = m_entityManager.create();
    m_entityMap[entity.getName()] = entity;
    return entity;
}
