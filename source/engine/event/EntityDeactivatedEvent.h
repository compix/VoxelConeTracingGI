#pragma once
#include "engine/ecs/EntityManager.h"

struct EntityDeactivatedEvent
{
    EntityDeactivatedEvent(const Entity& entity)
        :entity(entity) {}

    Entity entity;
};
