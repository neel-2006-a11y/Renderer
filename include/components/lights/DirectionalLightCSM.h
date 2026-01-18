#pragma once
#include "Component.h"
#include "Object.h"
#include "shadowCascade.h"
#include "camera.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class DirectionalLightCSM : public Component {
    public:
    // ===== light data =====
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    float halfRange = 100.0f;

    // ===== shadow settings =====
    bool castShadows = true;

    // ===== Cascaded ShadowMap Settings =====
    static const int NUM_CASCADES = 3;
    ShadowCascade cascades[NUM_CASCADES];

    float lambda = 0.5f; // split blending(0=linear, 1=logarithmic)

    DirectionalLightCSM(const glm::vec3& col = glm::vec3(1.0f), float intens = 1.0f)
            : color(col), intensity(intens) {}

    glm::vec3 getDirection() const;
};

void computeCascadeSplits(DirectionalLightCSM& light, float camNear, float camFar);
void updateCascadeLightMatrices(DirectionalLightCSM& light, const Camera& cam);