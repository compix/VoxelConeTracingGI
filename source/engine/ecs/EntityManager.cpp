#include "EntityManager.h"
#include "engine/event/event.h"
#include "engine/event/EntityDeactivatedEvent.h"
#include "engine/event/EntityActivatedEvent.h"

ComponentTypeID EntityManager::s_componentTypeIDCounter = 0;

// ******************** EntityManager Implementations ********************
void EntityManager::destroy(const Entity& entity)
{
    assert(valid(entity));

    // Call the destructors of the components
    for (size_t i = 0; i < m_componentPools.size(); ++i)
    {
        auto pool = m_componentPools[i];
        if (pool && m_componentMasks[i].test(i))
            pool->destroy(entity.m_id);
    }

    m_versions[entity.m_id]++;
    m_componentMasks[entity.m_id].reset();
    m_alive[entity.m_id] = false;
    m_active[entity.m_id] = false;
    m_idManager.release(entity.m_id);
}

std::vector<ComponentPtr<Component>> EntityManager::getAllComponents(const Entity& entity)
{
    assert(entity);

    std::vector<ComponentPtr<Component>> components;

    for (size_t componentTypeID = 0; componentTypeID < m_componentPools.size(); ++componentTypeID)
    {
        // Check if the entity has this component
        if (m_componentMasks[entity.m_id].test(componentTypeID))
            components.push_back(ComponentPtr<Component>(entity, componentTypeID));
    }

    return components;
}

void EntityManager::removeComponent(const Entity& entity, size_t componentTypeID)
{
    assert(valid(entity) && hasComponent(entity, componentTypeID));
    m_componentMasks[entity.m_id].reset(componentTypeID);
    getPool(componentTypeID)->destroy(entity.m_id);
}

const std::string& EntityManager::getName(const Entity& entity) const
{
    assert(entity);
    return m_names[entity.m_id];
}

void EntityManager::setActive(const Entity& entity, bool active)
{
    assert(entity);
    bool wasActive = m_active[entity.m_id];
    m_active[entity.m_id] = active;

    if (wasActive && !active)
        Event::transmit<EntityDeactivatedEvent>(entity);
    else if (!wasActive && active)
        Event::transmit<EntityActivatedEvent>(entity);
}

bool EntityManager::isActive(const Entity& entity)
{
    assert(entity);
    return m_active[entity.m_id];
}

bool EntityManager::hasComponent(const Entity& entity, size_t componentTypeID) const
{
    assert(valid(entity));
    return m_componentMasks[entity.m_id].test(componentTypeID);
}

const Component* EntityManager::getComponentPtr(const Entity& entity, size_t componentTypeID) const
{
    assert(valid(entity) && hasComponent(entity, componentTypeID));
    return getPool(componentTypeID)->getPtr(entity.m_id);
}

Component* EntityManager::getComponentPtr(const Entity& entity, size_t componentTypeID)
{
    assert(valid(entity) && hasComponent(entity, componentTypeID));
    return getPool(componentTypeID)->getPtr(entity.m_id);
}

Pool<Component>* EntityManager::getPool(size_t componentTypeID)
{
    assert(componentTypeID < m_componentPools.size());
    assert(m_componentPools[componentTypeID]);
    return static_cast<Pool<Component>*>(m_componentPools[componentTypeID]);
}

const Pool<const Component>* EntityManager::getPool(size_t componentTypeID) const
{
    assert(componentTypeID < m_componentPools.size());
    assert(m_componentPools[componentTypeID]);
    return static_cast<const Pool<const Component>*>(m_componentPools[componentTypeID]);
}

EntityManager::~EntityManager()
{
    for (auto pool : m_componentPools)
        if (pool)
            delete pool;
}

void EntityManager::update()
{
    for (size_t componentTypeID = 0; componentTypeID < m_componentPools.size(); ++componentTypeID)
    {
        auto componentPool = m_componentPools[componentTypeID];
        ComponentMask mask;
        mask.set(componentTypeID);

        if (componentPool)
        {
            for (size_t i = 0; i < componentPool->size(); ++i)
            {
                Component* c = reinterpret_cast<Component*>(componentPool->get(i));

                if ((m_componentMasks[i] & mask) == mask && m_alive[i] && m_active[i])
                {
                    c->update();
                }
            }
        }
    }
}

void EntityManager::lateUpdate()
{
    for (size_t componentTypeID = 0; componentTypeID < m_componentPools.size(); ++componentTypeID)
    {
        auto componentPool = m_componentPools[componentTypeID];
        ComponentMask mask;
        mask.set(componentTypeID);

        if (componentPool)
        {
            for (size_t i = 0; i < componentPool->size(); ++i)
            {
                Component* c = reinterpret_cast<Component*>(componentPool->get(i));

                if ((m_componentMasks[i] & mask) == mask && m_alive[i] && m_active[i])
                {
                    c->lateUpdate();
                }
            }
        }
    }
}

Entity EntityManager::create() { return create("Entity" + std::to_string(m_totalEntityCounter)); }

Entity EntityManager::create(const std::string& name)
{
    ++m_totalEntityCounter;
    EntityID id = m_idManager.next();

    // Make sure there is enough space for the newly created entity
    if (m_versions.size() <= id)
    {
        m_names.resize(id + 1);
        m_versions.resize(id + 1);
        m_versions[id] = 1;
        m_alive.resize(id + 1);
        m_active.resize(id + 1);
        m_componentMasks.resize(id + 1);

        // Reserve space for each component type
        for (auto pool : m_componentPools)
            if (pool)
                pool->resize(id + 1);
    }

    m_alive[id] = true;
    m_active[id] = true;
    m_names[id] = name;

    return Entity(id, m_versions[id], this);
}

// ******************** EntityIDManager Implementations ********************
EntityID EntityIDManager::next()
{
    if (m_freeIDs.size() > 0)
    {
        EntityID id = m_freeIDs.back();
        m_freeIDs.pop_back();
        return id;
    }

    return m_idCounter++;
}

void EntityIDManager::release(EntityID id)
{
    // Assert that this id is within the range of managed ids
    assert(id < m_idCounter);
    // Assert that this id hasn't been released yet
    assert(std::find(m_freeIDs.begin(), m_freeIDs.end(), id) == m_freeIDs.end());

    m_freeIDs.push_back(id);
}

// ******************** Entity Implementations ********************
Entity::Entity()
    : m_id(0), m_version(0), m_manager(nullptr) {}

Entity::Entity(EntityID id, EntityVersion version, EntityManager* manager)
    : m_id(id), m_version(version), m_manager(manager) {}

bool Entity::valid() const { return m_manager && m_manager->valid(*this); }

void Entity::destroy() const
{
    assert(valid() && "The entity is invalid.");
    m_manager->destroy(*this);
}

std::vector<ComponentPtr<Component>> Entity::getAllComponents() const
{
    assert(valid() && "The entity is invalid.");
    return m_manager->getAllComponents(*this);
}

const std::string& Entity::getName() const { return m_manager->getName(*this); }

void Entity::setActive(bool active) const { m_manager->setActive(*this, active); }

bool Entity::isActive() const { return m_manager->isActive(*this); }

ComponentPtr<Component>::ComponentPtr(const Entity& owner, size_t typeID)
    : m_owner(owner), m_typeID(typeID)
{
#if defined(DEBUG) || defined(_DEBUG)
	m_cPtr = valid() ? m_owner.m_manager->getComponentPtr(m_owner, m_typeID) : nullptr;
#endif
}

bool ComponentPtr<Component>::valid() const { return m_owner && m_owner.m_manager->hasComponent(m_owner, m_typeID); }

Component* ComponentPtr<Component>::operator->()
{
#if defined(DEBUG) || defined(_DEBUG)
	m_cPtr = m_owner.m_manager->getComponentPtr(m_owner, m_typeID);
#endif
    return m_owner.m_manager->getComponentPtr(m_owner, m_typeID);
}

const Component* ComponentPtr<Component>::operator->() const { return m_owner.m_manager->getComponentPtr(m_owner, m_typeID); }

void ComponentPtr<Component>::remove() const { m_owner.m_manager->removeComponent(m_owner, m_typeID); }
