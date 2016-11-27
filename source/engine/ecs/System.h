#pragma once
class EntityManager;

class System
{
public:
    virtual ~System() { }

    virtual void update(EntityManager& entityManager) = 0;
    virtual void lateUpdate(EntityManager& entityManager) {}
};
