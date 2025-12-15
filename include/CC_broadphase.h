#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

struct AABB_GPU {
    glm::vec4 min;
    glm::vec4 max;
};

struct CollisionPair {
    uint32_t a;
    uint32_t b;
};