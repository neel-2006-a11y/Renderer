#pragma once
#include "Component.h"
#include <glm/glm.hpp>

class Camera : public Component {
public:
    float fovY = 60.0f;          // degrees
    float nearPlane = 0.4f;
    float farPlane  = 70.0f;

    bool orthographic = false;
    float orthoHeight = 10.0f;

    // Cached
    mutable glm::mat4 view{1.0f};
    mutable glm::mat4 projection{1.0f};

    mutable bool viewDirty = true;
    mutable bool projDirty = true;

    glm::mat4 getView() const;
    glm::mat4 getProjection() const;
    glm::mat4 getVP() const;
};
