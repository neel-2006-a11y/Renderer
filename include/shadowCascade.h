#pragma once
#include "glm/glm.hpp"

#include "ShadowMap.h"

struct ShadowCascade {
    glm::mat4 lightVP;
    float splitNear;
    float splitFar;
};