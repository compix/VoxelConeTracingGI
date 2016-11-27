#pragma once
#include <glm/glm.hpp>
#include <engine/geometry/Transform.h>
#include <glm/ext.hpp>
#include <engine/util/math.h>
#include "Command.h"

/**
* Rotates a transform from a start rotation to a destination rotation within a given time constraint.
*/
struct RotationCommand : Command
{
    RotationCommand() {}

    RotationCommand(const ComponentPtr<Transform>& transform, const glm::quat& startRotation, const glm::quat& destinationRotation, float duration = 1.0f)
        : transform(transform), startRotation(startRotation), destinationRotation(destinationRotation), duration(duration)
    {
        if (this->duration <= 0.0f)
            this->duration = math::EPSILON;
    }

    void operator()(float deltaTime) override
    {
        if (t >= 1.0f || !transform)
            return;

        t += deltaTime / duration;
        glm::quat curRotation = glm::slerp(startRotation, destinationRotation, math::clamp(t, 0.0f, 1.0f));
        transform->setRotation(curRotation);
    }

    bool done() const override { return t >= 1.0f || !transform; }

    void reset() override { t = 0.0f; }

    ComponentPtr<Transform> transform;
    glm::quat startRotation;
    glm::quat destinationRotation;
    float t{ 0.0f };
    float duration{ 1.0f };
};
