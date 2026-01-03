#pragma once
#include "glm/glm.hpp"

#include "ShadowMap.h"

struct ShadowCascade {
    ShadowMap shadowMap;
    glm::mat4 lightVP;
    float splitNear;
    float splitFar;
};