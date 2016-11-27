#pragma once
#include <glm/glm.hpp>
#include <engine/util/math.h>

struct OBBox
{
    glm::mat4 model{math::identityMatrix()}; // Has the 3 axes and center position
    glm::vec3 he; // half-extents vector
};
