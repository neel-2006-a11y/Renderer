#include "CameraController.h"
#include "Object.h"
#include "transform.h"
#include "camera.h"
#include "app.h"

#include <SDL2/SDL.h>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>

void CameraController::start() {
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void CameraController::update(float dt) {
    handleMouse();
    handleKeyboard(dt);

    // Update transform rotation from yaw/pitch
    pitch = std::clamp(pitch, -1.55f, 1.55f);

    glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1,0,0));
    glm::quat qYaw   = glm::angleAxis(yaw,   glm::vec3(0,1,0));

    owner->transform->rotation = qYaw * qPitch;

    // Mark camera dirty
    if (auto cam = owner->getComponent<Camera>()) {
        cam->viewDirty = true;
    }
}

void CameraController::handleKeyboard(float dt) {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    float speed = moveSpeed;
    if (keys[SDL_SCANCODE_LSHIFT]) // sprint
        speed *= 2.5f;

    Transform* t = owner->transform;

    glm::vec3 forward = t->forward();
    glm::vec3 right   = t->right();
    glm::vec3 up      = t->up();

    if (keys[SDL_SCANCODE_W]) t->position += forward * speed * dt;
    if (keys[SDL_SCANCODE_S]) t->position -= forward * speed * dt;
    if (keys[SDL_SCANCODE_A]) t->position -= right   * speed * dt;
    if (keys[SDL_SCANCODE_D]) t->position += right   * speed * dt;
    if (keys[SDL_SCANCODE_E]) t->position += up      * speed * dt;
    if (keys[SDL_SCANCODE_Q]) t->position -= up      * speed * dt;
}

void CameraController::handleMouse() {
    App& app = App::instance();
    if(!app.relativeMouseMode)return;
    
    int dx, dy;
    SDL_GetRelativeMouseState(&dx, &dy);

    yaw   -= dx * mouseSensitivity;
    pitch -= dy * mouseSensitivity;
}

