#pragma once
#include "engine/ecs/EntityManager.h"

struct EntityActivatedEvent
{
    EntityActivatedEvent(const Entity& entity)
        :entity(entity) {}

    Entity entity;
};
