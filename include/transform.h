#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Component.h"

class Transform : public Component {
public:
    glm::vec3 position{0};
    glm::quat rotation{1,0,0,0};
    glm::vec3 scale{1,1,1};

    glm::mat4 getMatrix() const {
        glm::mat4 T = glm::translate(glm::mat4(1), position);
        glm::mat4 R = glm::mat4_cast(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1), scale);
        return T * R * S;
    }
};
