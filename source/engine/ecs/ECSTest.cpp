#include "ECSTest.h"
#include <cstdint>
#include <cassert>
#include "EntityManager.h"
#include "ecs.h"

namespace ecs_test
{
    struct PositionComponent : public Component
    {
        PositionComponent(int x, int y)
            : x(x), y(y) { }

        int x, y;
    };

    struct VelocityComponent : public Component
    {
        VelocityComponent(int x, int y)
            : x(x), y(y) { }

        int x, y;
    };

    struct RenderComponent : public Component
    {
        float someValue = 0.f;
        uint16_t usefulValue = 10;
    };

    struct AnimationComponent : public Component
    {
        float time = 0.f;
    };

    class PhysicsSystem : public System
    {
    public:
        void update(EntityManager& manager) override
        {
            for (Entity entity : manager.getEntitiesWithComponents<PositionComponent, VelocityComponent>())
            {
                auto posComponent = entity.getComponent<PositionComponent>();
                auto velocityComponent = entity.getComponent<VelocityComponent>();

                posComponent->x += velocityComponent->x;
                posComponent->y += velocityComponent->y;
            }
        }
    };

    template <class... Components>
    void assertNumberOfEntitiesWithComponents(EntityManager& entityManager, int size)
    {
        assert(int(entityManager.numberOfEntitiesWithComponents<Components...>()) == size);
    }

    template <class T1, class T2>
    bool allEqual(const T1& v1, const T2& v2) { return v1 == v2; }

    template <class T1, class T2, class... Args>
    bool allEqual(const T1& v1, const T2& v2, const Args& ... args) { return v1 == v2 && allEqual(v2, args...); }

    template <class T1, class T2>
    bool allNotEqual(const T1& v1, const T2& v2) { return v1 != v2; }

    template <class T1, class T2, class... Args>
    bool allNotEqual(const T1& v1, const T2& v2, const Args& ... args) { return v1 != v2 && allNotEqual(v2, args...) && allNotEqual(v1, args...); }

    template <class T1, class T2, class... Args>
    bool allDifferent(const T1& v1, const T2& v2, const Args& ... args) { return allNotEqual(v1, v2, args...); }

#ifdef RUN_ECS_TESTS
    struct ECSTestRunner
    {
        ECSTestRunner()
        {
            ECSTest::runTests();
        }
    };

    ECSTestRunner ecsTestRunner;
#endif
}

using namespace ecs_test;

void ECSTest::runTests()
{
    testComponentTypeID();
    testEntityCreateDestroyValid();
    testHasGetAddRemoveComponent();
    testGetEntitiesWithComponents();
    testSystem();
}

void ECSTest::testComponentTypeID()
{
    // Make 2 tests with 2 different EntityManager instances where in the second test a new component comes in
    // to ensure that the type id doesn't break functionality with multiple instances. They should all be different.
    {
        // Setup a test environment
        EntityManager entityManager;

        assert(allDifferent(
                entityManager.getComponentTypeID<PositionComponent>(),
            entityManager.getComponentTypeID<VelocityComponent>(),
            entityManager.getComponentTypeID<RenderComponent>()));
    }

    {
        // Setup a test environment
        EntityManager entityManager;

        assert(allDifferent(
                entityManager.getComponentTypeID<PositionComponent>(),
            entityManager.getComponentTypeID<VelocityComponent>(),
            entityManager.getComponentTypeID<RenderComponent>(),
            entityManager.getComponentTypeID<AnimationComponent>()));
    }
}

void ECSTest::testGetEntitiesWithComponents()
{
    // Setup a test environment
    EntityManager entityManager;

    int entitySize = 100;

    for (int i = 0; i < entitySize; ++i)
    {
        Entity entity = entityManager.create();
        entity.addComponent<PositionComponent>(i, 0);
        entity.addComponent<VelocityComponent>(0, 1);

        if (i % 2 == 0)
            entity.addComponent<RenderComponent>();
    }

    assertNumberOfEntitiesWithComponents<RenderComponent>(entityManager, entitySize / 2);
    assertNumberOfEntitiesWithComponents<AnimationComponent>(entityManager, 0);
    assertNumberOfEntitiesWithComponents<PositionComponent, VelocityComponent>(entityManager, entitySize);
    assertNumberOfEntitiesWithComponents<PositionComponent>(entityManager, entitySize);
    assertNumberOfEntitiesWithComponents<VelocityComponent>(entityManager, entitySize);
}

void ECSTest::testEntityCreateDestroyValid()
{
    // Setup a test environment
    EntityManager entityManager;
    Entity entity = entityManager.create();
    assert(entity.valid() && entity);

    entity.destroy();
    assert(!entity && !entity.valid());

    Entity entity2 = entityManager.create();
    assert(entity2.m_id == 0);
    assert(entity2.valid() && entity2);
    assert(!entity && !entity.valid() && "Old Entity that has been destroyed is valid after index reuse.");
    assert(!Entity() && !Entity().valid() && "Default constructed Entity should not be valid.");
}

void ECSTest::testHasGetAddRemoveComponent()
{
    // Setup a test environment
    EntityManager entityManager;
    Entity entity = entityManager.create();
    assert(
        !(entity.hasComponent<PositionComponent>() &&
        entity.hasComponent<RenderComponent>() &&
        entity.hasComponent<AnimationComponent>() &&
        entity.hasComponent<VelocityComponent>()));

    entity.addComponent<PositionComponent>(1, 2);
    entity.addComponent<RenderComponent>();
    entity.addComponent<AnimationComponent>();
    assert(entity.hasComponent<PositionComponent>() && entity.hasComponent<RenderComponent>() && entity.hasComponent<AnimationComponent>());

    ComponentPtr<PositionComponent> position = entity.getComponent<PositionComponent>();
    assert(position && position->x == 1.f && position->y == 2.f);
    assert(entity.getComponent<AnimationComponent>() && entity.getComponent<RenderComponent>());

    entity.removeComponent<PositionComponent>();
    assert(!entity.hasComponent<PositionComponent>());
    assert(entity.hasComponent<RenderComponent>() && entity.hasComponent<AnimationComponent>());
    entity.removeComponent<AnimationComponent>();
    assert(!entity.hasComponent<PositionComponent>() && !entity.hasComponent<AnimationComponent>());
    assert(entity.hasComponent<RenderComponent>());
    entity.removeComponent<RenderComponent>();
    assert(!entity.hasComponent<PositionComponent>() && !entity.hasComponent<AnimationComponent>() && !entity.hasComponent<RenderComponent>());

    assert(!entity.getComponent<PositionComponent>() && !entity.getComponent<RenderComponent>() && !entity.getComponent<AnimationComponent>());
}

void ECSTest::testSystem()
{
    // Setup a test environment
    ECS::addSystem<PhysicsSystem>();

    int entitySize = 100;
    int numIterations = 100;

    for (int i = 0; i < entitySize; ++i)
    {
        Entity entity = ECS::createEntity();
        entity.addComponent<PositionComponent>(0, 0);
        entity.addComponent<VelocityComponent>(0, 1);

        if (i % 2 == 0)
            entity.addComponent<RenderComponent>();
    }

    for (int i = 0; i < numIterations; ++i)
        ECS::update();

    for (Entity entity : ECS::getEntitiesWithComponents<PositionComponent>())
    {
        assert(entity.getComponent<PositionComponent>()->y == numIterations);
        assert(entity.getComponent<PositionComponent>()->x == 0.f);
    }
}
