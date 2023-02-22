#pragma once
#include <vector>
#include <memory>
#include "System.h"
#include <typeindex>
#include <unordered_map>
#include <cassert>
#include "EntityManager.h"
#include <cstddef>

class ECS
{
public:
    template <class S, class... Args>
    static void addSystem(Args&&... args);

    template <class S>
    static void removeSystem();

    static void update();
    static void lateUpdate();

    static Entity createEntity(const std::string& name);
    static Entity createEntity();

    static Entity getEntityByName(const std::string& name) { return m_entityMap[name]; }

    static Entity getEntity(EntityID id, EntityVersion version) { return Entity(id, version, &m_entityManager); }

    template <class... Components>
    static EntityManager::EntityIterator getEntitiesWithComponents()
    {
        return m_entityManager.getEntitiesWithComponents<Components...>();
    }

    template <class... Components>
    static EntityManager::EntityIteratorIncludeInactive getEntitiesWithComponentsIncludeInactive()
    {
        return m_entityManager.getEntitiesWithComponentsIncludeInactive<Components...>();
    }

    template <class... Components>
    static std::size_t getEntityCountWithComponents();

    template <class... Components>
    static std::size_t getEntityCountWithComponentsIncludeInactive();
private:
    static EntityManager m_entityManager;
    static std::unordered_map<std::type_index, System*> m_systems;
    static std::unordered_map<std::string, Entity> m_entityMap; // Maps entities by names
};

template <class S, class... Args>
void ECS::addSystem(Args&&... args)
{
    assert(m_systems.count(typeid(S)) == 0);

    m_systems[typeid(S)] = new S(std::forward<Args>(args)...);
}

template <class S>
void ECS::removeSystem()
{
    assert(m_systems.count(typeid(S)) > 0);
    System* system = m_systems[typeid(S)];
    m_systems.erase(typeid(S));
    delete system;
}

template <class ... Components>
std::size_t ECS::getEntityCountWithComponents()
{
    std::size_t count = 0;

    for (auto e : m_entityManager.getEntitiesWithComponents<Components...>())
    {
        ++count;
    }

    return count;
}

template <class ... Components>
std::size_t ECS::getEntityCountWithComponentsIncludeInactive()
{
    std::size_t count = 0;

    for (auto e : m_entityManager.getEntitiesWithComponentsIncludeInactive<Components...>())
    {
        ++count;
    }

    return count;
}
