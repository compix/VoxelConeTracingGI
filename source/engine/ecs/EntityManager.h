#pragma once
#include <vector>
#include <bitset>
#include <limits>
#include <assert.h>
#include "ecs_settings.h"
#include <engine/memory/Pool.h>
#include <unordered_map>
#include <cstddef>

/*
~~~~~ Features ~~~~~
- Good cache utilization.
- Fast Add, Remove, Get operations on entities and components.
- Fast entity traversal.

~~~~~ Limitations ~~~~~
- No multithreading considerations.
- High space reservation. Example:
100 entities are in the game - only 1 entity has Component of type X - space for 100 components of types X is reserved.

~~~~~ Future considerations ~~~~~
- Make it hybrid:
There is often no need for fast Add, Remove operations but a higher need for lower space consumption.
Give the user the ability to specify usage hints for component types:
- "Mostly Static": No or very low Add, Remove usage.
-> lower space consumption (O(n) where n is the number of components) but slower Add, Remove
- "Highly Dynamic": High usage of Add, Remove operations.
-> faster Add, Remove but higher space consumption (O(n) where n is the number of entities)
- "Default": One of the following ideas: (TBD)
1. Compromise between the 2 above: Reserve "a bit" (TBD) more space to prepare for Add, Remove
or
2. Adapt to usage (intelligent/learning system) - Implementation time cost probably outweighs the (potentially low) benefits.
or
3. Default to Static or Dynamic
*/

class Component;
template <class C>
class ComponentPtr;

// Used to identify entities within an engine instance
// This id is used as an index into the entity container
using EntityID = uint32_t;

// Used to record the version of entities to determine validity.
// An entity with the version 0 is invalid. The version of a newly created entity is either
// 1 or a higher number if the entity ID is recycled (= it was already in use, destroyed later and now recycled).
// If an entity is destroyed then the internal version number is increased. The entity with an old version is thus invalid.
using EntityVersion = uint32_t;

class EntityManager;

/**
* An Entity is anything that exists in the game world
* - visible: Players, Enemies, Cars... or invisible: EventTriggers, Camera Sequences...
* Zero or more components are assigned to each entity.
* A default-constructed and destroyed entity is invalid.
*
* One or more components declare an atomic functionality, property, characteristic or behavior.
* Components provide all the data which is necessary to define the functionality.
*
* A System defines the functionality that is implicitly provided by the components.
*/
class Entity
{
    friend class EntityManager;
    friend class ECSTest;
    friend class Component;
    template <class C>
    friend class ComponentPtr;
public:
    Entity();
    Entity(EntityID id, EntityVersion version, EntityManager* manager);

    bool valid() const;

    operator bool() const { return valid(); }

    bool operator==(const Entity& entity) const { return m_id == entity.m_id && m_version == entity.m_version && m_manager == entity.m_manager; }

    bool operator==(Entity& entity) const { return m_id == entity.m_id && m_version == entity.m_version && m_manager == entity.m_manager; }

    void destroy() const;

    template <class C>
    void removeComponent();

    template <class C, class... Args>
    void addComponent(Args&&... args);

    template <class C>
    bool hasComponent() const;

    template <class C, class... Components>
    bool hasComponents() const;

    template <class C>
    ComponentPtr<C> getComponent();

    std::vector<ComponentPtr<Component>> getAllComponents() const;

    const std::string& getName() const;

    void setActive(bool active) const;
    bool isActive() const;

    EntityID getID() const { return m_id; }

    EntityVersion getVersion() const { return m_version; }

private:
    template <class C>
    C* getComponentPtr();

    template <class C>
    const C* getComponentPtr() const;

private:
    EntityID m_id;
    EntityVersion m_version;
    EntityManager* m_manager;
};

/**
* Provides access to components.
*/
template <class C>
class ComponentPtr
{
public:
    ComponentPtr() { }

    explicit ComponentPtr(const Entity& owner);

    bool operator ==(const ComponentPtr<C>& other) const { return m_owner == other.m_owner; }

    bool valid() const;

    operator bool() const { return valid(); }

    C* operator ->();
    const C* operator ->() const;

    void remove();

    Entity getOwner() const { return m_owner; }

private:
    Entity m_owner;

    // Allows to see the contents of the component when debugging
    // Note: This can potentially have incorrect values after 
    // some space moving/reallocation operations in the EntityManager/Pools take place
    // e.g. after adding or removing new components.
    // This is the reason the value is updated in the -> operator.
#if defined(DEBUG) || defined(_DEBUG)
	const C* m_cPtr{ nullptr };
#endif
};

// Need a specialization for the base type
template <>
class ComponentPtr<Component>
{
public:
    ComponentPtr() { }

    explicit ComponentPtr(const Entity& owner, std::size_t typeID);

    bool operator ==(const ComponentPtr<Component>& other) const { return m_owner == other.m_owner && m_typeID == other.m_typeID; }

    bool valid() const;

    operator bool() const { return valid(); }

    Component* operator ->();
    const Component* operator ->() const;

    void remove() const;

    Entity getOwner() const { return m_owner; }

private:
    Entity m_owner;
    std::size_t m_typeID{std::numeric_limits<std::size_t>().max()};

    // Allows to see the contents of the component when debugging
    // Note: This can potentially have incorrect values after 
    // some space moving/reallocation operations in the EntityManager/Pools take place
    // e.g. after adding or removing new components.
    // This is the reason the value is updated in the -> operator.
#if defined(DEBUG) || defined(_DEBUG)
	const Component* m_cPtr{ nullptr };
#endif
};

class Component
{
    friend class EntityManager;
public:
    virtual ~Component() { }

    virtual void update() { }
    virtual void lateUpdate() { }

    virtual void onShowInEditor() { }

    virtual std::string getName() const { return "UNKNOWN_COMPONENT"; }

    Entity getOwner() const { return m_owner; }

    template <class C>
    void removeComponent();

    template <class C, class... Args>
    void addComponent(Args&&... args);

    template <class C>
    bool hasComponent() const;

    template <class C, class... Components>
    bool hasComponents() const;

    template <class C>
    ComponentPtr<C> getComponent() const;
protected:
    Entity m_owner;
};

class EntityIDManager
{
public:
    EntityID next();
    void release(EntityID id);

private:
    EntityID m_idCounter{0};
    std::vector<EntityID> m_freeIDs;
};

// Container for the component pools
// Each Pool manages components of a unique component type
using ComponentPools = std::vector<BasePool*>;

// The component mask is used to identify which components are assigned to an entity
// All component types have one unique bit which corresponds to the ComponentPools index
using ComponentMask = std::bitset<MAX_COMPONENTS>;
using ComponentMasks = std::vector<ComponentMask>;

using EntityVersions = std::vector<EntityVersion>;

using ComponentTypeID = uint32_t;

class EntityManager
{
    friend class Entity;
    friend class ECSTest;
    friend class ECS;
    friend class ComponentPtr<Component>;

    // Considers entities as valid even if they are inactive
    class ValidTypeIncludeInactive;
    // Default ValidType: inactive entities will be skipped
    class ValidTypeDefault;

    template <class TValid>
    class EntityIteratorBase : public std::iterator<std::input_iterator_tag, Entity>
    {
        friend class EntityManager;

    public:
        bool operator ==(const EntityIteratorBase& other) const { return m_i == other.m_i; }

        bool operator !=(const EntityIteratorBase& other) const { return m_i != other.m_i; }

        Entity operator *() { return Entity(m_i, m_manager->m_versions[m_i], m_manager); }

        Entity operator *() const { return Entity(m_i, m_manager->m_versions[m_i], m_manager); }

        EntityIteratorBase& operator ++()
        {
            ++m_i;
            nextValid();
            return *this;
        }

        EntityIteratorBase begin() const
        {
            auto it = EntityIteratorBase(m_manager, m_mask, 0);
            it.nextValid();
            return it;
        }

        EntityIteratorBase end() const { return EntityIteratorBase(m_manager, m_mask, m_manager->capacity()); }

    private:
        EntityIteratorBase(EntityManager* manager, const ComponentMask& mask, EntityID i = 0)
            : m_manager(manager), m_mask(mask), m_i(i) { }

        void nextValid()
        {
            while (m_i < m_manager->m_alive.size() && !valid())
                ++m_i;
        }

        template <class T = TValid>
        typename std::enable_if<std::is_same<T, ValidTypeDefault>::value, bool>::type valid() const { return m_manager->m_alive[m_i] && m_manager->m_active[m_i] && (m_manager->m_componentMasks[m_i] & m_mask) == m_mask; }

        template <class T = TValid>
        typename std::enable_if<std::is_same<T, ValidTypeIncludeInactive>::value, bool>::type valid() const { return m_manager->m_alive[m_i] && (m_manager->m_componentMasks[m_i] & m_mask) == m_mask; }

    private:
        EntityManager* m_manager;
        ComponentMask m_mask;
        EntityID m_i;
    };

    using EntityIterator = EntityIteratorBase<ValidTypeDefault>;
    using EntityIteratorIncludeInactive = EntityIteratorBase<ValidTypeIncludeInactive>;

public:
    ~EntityManager();

    void update();
    void lateUpdate();

    bool valid(const Entity& entity) const { return entity.m_id < m_versions.size() && m_versions[entity.m_id] == entity.m_version; }

    Entity create();
    Entity create(const std::string& name);
    void destroy(const Entity& entity);

    std::vector<ComponentPtr<Component>> getAllComponents(const Entity& entity);

    template <class C, class... Args>
    void addComponent(const Entity& entity, Args&&... args);

    template <class C>
    void removeComponent(const Entity& entity);

    void removeComponent(const Entity& entity, std::size_t componentTypeID);

    template <class C>
    bool hasComponent(const Entity& entity) const;

    template <class C, class... Components>
    bool hasComponents(const Entity& entity) const;

    template <class C>
    ComponentPtr<C> getComponent(const Entity& entity) const;

    const std::string& getName(const Entity& entity) const;

    void setActive(const Entity& entity, bool active);
    bool isActive(const Entity& entity);

    template <class... Components>
    EntityIterator getEntitiesWithComponents();

    template <class... Components>
    EntityIteratorIncludeInactive getEntitiesWithComponentsIncludeInactive();

    template <class... Components>
    EntityID numberOfEntitiesWithComponents(bool includeInactive = true);
private:
    bool hasComponent(const Entity& entity, std::size_t componentTypeID) const;

    const Component* getComponentPtr(const Entity& entity, std::size_t componentTypeID) const;
    Component* getComponentPtr(const Entity& entity, std::size_t componentTypeID);

    EntityID capacity() const { return EntityID(m_versions.size()); }

    template <class C>
    C* getComponentPtr(const Entity& entity);

    template <class C>
    const C* getComponentPtr(const Entity& entity) const;

    template <class C>
    Pool<C>* getPool();

    Pool<Component>* getPool(std::size_t componentTypeID);

    template <class C>
    const Pool<const C>* getPool() const;

    const Pool<const Component>* getPool(std::size_t componentTypeID) const;

    // Used to assign an id to each component type local to the EntityManager.
    // Starting at 0 and increasing by 1. It's used as an index into a container.
    template <class C>
    static ComponentTypeID getComponentTypeID();

    template <class C>
    ComponentMask makeComponentMask() const;

    template <class C1, class C2, class... Cn>
    ComponentMask makeComponentMask() const;
private:
    static ComponentTypeID s_componentTypeIDCounter;
    std::vector<bool> m_alive;
    std::vector<bool> m_active;
    ComponentPools m_componentPools;
    ComponentMasks m_componentMasks;
    std::vector<std::string> m_names;
    EntityVersions m_versions;
    EntityIDManager m_idManager;
    std::size_t m_totalEntityCounter{0}; // Increases when a new entity is added but never decreases
};

template <class C, class ... Args>
void EntityManager::addComponent(const Entity& entity, Args&&... args)
{
    static_assert(std::is_base_of<Component, C>::value, "EntityManager::addComponent: All components must derive from Component.");

    ComponentTypeID compTypeID = getComponentTypeID<C>();

    // Make sure a component pool for this component type exists
    if (m_componentPools.size() <= compTypeID) { m_componentPools.resize(compTypeID + 1); }

    if (!m_componentPools[compTypeID])
    {
        m_componentPools[compTypeID] = new Pool<C>();
        m_componentPools[compTypeID]->resize(m_versions.size());
    }

    // Set the component mask bit
    m_componentMasks[entity.m_id].set(compTypeID);

    // Create the component with the given arguments
    C* component = getPool<C>()->create(entity.m_id, std::forward<Args>(args)...);
    component->m_owner = entity;
}

template <class C>
void EntityManager::removeComponent(const Entity& entity)
{
    assert(valid(entity) && hasComponent<C>(entity));
    m_componentMasks[entity.m_id].reset(getComponentTypeID<C>());
    getPool<C>()->destroy(entity.m_id);
}

template <class C>
bool EntityManager::hasComponent(const Entity& entity) const
{
    assert(valid(entity));
    return m_componentMasks[entity.m_id].test(getComponentTypeID<C>());
}

template <class C, class ... Components>
bool EntityManager::hasComponents(const Entity& entity) const
{
    assert(valid(entity));
    auto mask = makeComponentMask<C, Components...>();
    return (m_componentMasks[entity.m_id] & mask) == mask;
}

template <class C>
ComponentPtr<C> EntityManager::getComponent(const Entity& entity) const
{
    return ComponentPtr<C>(entity);
}

template <class ... Components>
EntityManager::EntityIterator EntityManager::getEntitiesWithComponents()
{
    return EntityIterator(this, makeComponentMask<Components...>());
}

template <class ... Components>
EntityManager::EntityIteratorIncludeInactive EntityManager::getEntitiesWithComponentsIncludeInactive()
{
    return EntityIteratorIncludeInactive(this, makeComponentMask<Components...>());
}

template <class ... Components>
EntityID EntityManager::numberOfEntitiesWithComponents(bool includeInactive)
{
    ComponentMask mask = makeComponentMask<Components...>();

    EntityID size = 0;
    if (includeInactive)
    {
        for (EntityID i = 0; i < m_versions.size(); ++i)
            if (m_alive[i] && (m_componentMasks[i] & mask) == mask)
                ++size;
    }
    else
    {
        for (EntityID i = 0; i < m_versions.size(); ++i)
            if (m_alive[i] && m_active[i] && (m_componentMasks[i] & mask) == mask)
                ++size;
    }

    return size;
}

template <class C>
C* EntityManager::getComponentPtr(const Entity& entity)
{
    assert(valid(entity) && hasComponent<C>(entity));
    return getPool<C>()->getPtr(entity.m_id);
}

template <class C>
const C* EntityManager::getComponentPtr(const Entity& entity) const
{
    assert(valid(entity) && hasComponent<C>(entity));
    return getPool<C>()->getPtr(entity.m_id);
}

template <class C>
Pool<C>* EntityManager::getPool()
{
    assert(getComponentTypeID<C>() < m_componentPools.size());
    assert(m_componentPools[getComponentTypeID<C>()]);
    return static_cast<Pool<C>*>(m_componentPools[getComponentTypeID<C>()]);
}

template <class C>
const Pool<const C>* EntityManager::getPool() const
{
    assert(getComponentTypeID<C>() < m_componentPools.size());
    assert(m_componentPools[getComponentTypeID<C>()]);
    return static_cast<const Pool<const C>*>(m_componentPools[getComponentTypeID<C>()]);
}

template <class C>
ComponentTypeID EntityManager::getComponentTypeID()
{
    static_assert(std::is_base_of<Component, C>(), "EntityManager::getComponentTypeID: All components must derive from Component.");

    static ComponentTypeID id = s_componentTypeIDCounter++;
    // If the id reaches MAX_COMPONENTS then MAX_COMPONENTS should be increased
    assert(id < MAX_COMPONENTS);
    return id;
}

template <class C>
ComponentMask EntityManager::makeComponentMask() const
{
    ComponentMask mask;
    mask.set(getComponentTypeID<C>());
    return mask;
}

template <class C1, class C2, class ... Cn>
ComponentMask EntityManager::makeComponentMask() const
{
    return makeComponentMask<C1>() | makeComponentMask<C2, Cn...>();
}

// ******************** Entity Implementations ********************
template <class C>
void Entity::removeComponent()
{
    assert(valid() && "The entity is invalid.");
    m_manager->removeComponent<C>(*this);
}

template <class C, class ... Args>
void Entity::addComponent(Args&&... args)
{
    assert(valid() && "The entity is invalid.");
    m_manager->addComponent<C>(*this, std::forward<Args>(args)...);
}

template <class C>
bool Entity::hasComponent() const
{
    assert(valid() && "The entity is invalid.");
    return m_manager->hasComponent<C>(*this);
}

template <class C, class ... Components>
bool Entity::hasComponents() const
{
    assert(valid() && "The entity is invalid.");
    return m_manager->hasComponents<C, Components...>(*this);
}

template <class C>
ComponentPtr<C> Entity::getComponent()
{
    assert(valid() && "The entity is invalid.");
    return m_manager->getComponent<C>(*this);
}

template <class C>
C* Entity::getComponentPtr()
{
    return m_manager->getComponentPtr<C>(*this);
}

template <class C>
const C* Entity::getComponentPtr() const
{
    return m_manager->getComponentPtr<C>(*this);
}


// ******************** ComponentPtr Implementations ********************
template <class C>
ComponentPtr<C>::ComponentPtr(const Entity& owner)
    : m_owner(owner)
{
#if defined(DEBUG) || defined(_DEBUG)
    m_cPtr = valid() ? m_owner.getComponentPtr<C>() : nullptr;
#endif
}

template <class C>
bool ComponentPtr<C>::valid() const
{
    return m_owner && m_owner.hasComponent<C>();
}

template <class C>
C* ComponentPtr<C>::operator->()
{
#if defined(DEBUG) || defined(_DEBUG)
    m_cPtr = m_owner.getComponentPtr<C>();
#endif
    return m_owner.getComponentPtr<C>();
}

template <class C>
const C* ComponentPtr<C>::operator->() const { return m_owner.getComponentPtr<C>(); 
}

template <class C>
void ComponentPtr<C>::remove()
{
    m_owner.removeComponent<C>();
}

template <class C>
void Component::removeComponent()
{
    m_owner.m_manager->removeComponent<C>(m_owner);
}

template <class C, class ... Args>
void Component::addComponent(Args&&... args)
{
    m_owner.m_manager->addComponent<C>(m_owner);
}

template <class C>
bool Component::hasComponent() const
{
    return m_owner.m_manager->hasComponent<C>(m_owner);
}

template <class C, class ... Components>
bool Component::hasComponents() const
{
    return m_owner.hasComponents<C, Components...>();
}

template <class C>
ComponentPtr<C> Component::getComponent() const
{
    return m_owner.m_manager->getComponent<C>(m_owner);
}
