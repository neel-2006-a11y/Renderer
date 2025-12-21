#pragma once
#include "Component.h"
#include "Object.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class DirectionalLight : public Component {
    public:
    // ===== light data =====
    glm::vec3 direction = glm::vec3(-1,-1,-1);
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

    DirectionalLight() = default;

    DirectionalLight(const glm::vec3& dir,
                     const glm::vec3& col = glm::vec3(1.0f),
                     float intens = 1.0f)
            : color(col), intensity(intens) {}


    void start() override{
        direction = owner->transform->forward();
    }

    void update(float /*dt*/) override{
        if(owner)
            direction = -owner->transform->forward();
    }


};