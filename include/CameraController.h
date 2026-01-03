#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <cmath>

class CameraController : public Component {
public:
    float moveSpeed = 6.0f;
    float mouseSensitivity = 0.002f;

    float yaw = -3.14159/2.0f;
    float pitch = 0.0f;

    void start() override;
    void update(float dt) override;

private:
    void handleKeyboard(float dt);
    void handleMouse();
};
