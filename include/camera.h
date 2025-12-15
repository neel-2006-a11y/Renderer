#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

    float deltaTime;
    float lastFrame;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw = -90.0f,
           float startPitch = 0.0f);

    glm::mat4 getViewMatrix() const;
    void processKeyboard(Camera_Movement direction);
    void processMouseMovement(float xOffset, float yOffset);
    void updateCameraVectors();
    void updateDeltaTime();
};
