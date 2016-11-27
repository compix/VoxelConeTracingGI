#pragma once
#include <glm/glm.hpp>
#include <engine/geometry/Transform.h>
#include <glm/ext.hpp>
#include <engine/util/math.h>
#include "Command.h"

/**
* Moves a transform from a start position to a destination position within a given time constraint.
*/
struct MoveCommand : Command
{
    MoveCommand() {}

    MoveCommand(const ComponentPtr<Transform>& transform, const glm::vec3& startPos, const glm::vec3& destinationPos, float duration = 1.0f)
        : transform(transform), startPos(startPos), destinationPos(destinationPos), duration(duration)
    {
        if (this->duration <= 0.0f)
            this->duration = math::EPSILON;
    }

    void operator()(float deltaTime) override
    {
        if (t >= 1.0f || !transform)
            return;

        t += deltaTime / duration;
        glm::vec3 curPos = glm::lerp(startPos, destinationPos, math::clamp(t, 0.0f, 1.0f));
        transform->setPosition(curPos);
    }

    bool done() const override { return t >= 1.0f || !transform; }

    void reset() override { t = 0.0f; }

    ComponentPtr<Transform> transform;
    glm::vec3 startPos;
    glm::vec3 destinationPos;
    float t{0.0f};
    float duration{1.0f};
};
