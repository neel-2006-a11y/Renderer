#pragma once
#include "Component.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class DirectionalLight : public Component {
    public:
    // ===== light data =====
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    // ===== shadow settings =====
    bool castShadows = true;

    // Orthographic shadow volume
    float shadowSize = 30.0f; // half-width of orthobox
    float shadowNear = -50.0f;
    float shadowFar = 50.0f;

    // Bias control 
    float depthBias = 0.0015f;
    float normalBias = 0.002f;

    // Resolution
    int shadowResolution = 2048;

    // Cached Matrices
    glm::mat4 lightView;
    glm::mat4 lightProj;
    glm::mat4 lightVP;

    DirectionalLight(const glm::vec3& col = glm::vec3(1.0f), float intens = 1.0f)
            : color(col), intensity(intens) {}

    glm::vec3 getDirection() const;
    void updateLightMatrices(const glm::vec3& sceneCenter);
    void updateLightMatrices(const std::vector<glm::vec3>& frustumCorners);
};