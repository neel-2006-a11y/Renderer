#pragma once
#include <glm/glm.hpp>
#include <cmath>

// OBB
glm::mat3 computeInertiaTensorOBB(const glm::vec3& halfExtents, float mass) {
    glm::mat3 I(0.0f);
    float factor = (1.0f / 12.0f) * mass;

    float wx = 2.0f * halfExtents.x;
    float wy = 2.0f * halfExtents.y;
    float wz = 2.0f * halfExtents.z;
    I[0][0] = factor * (wy * wy + wz * wz);
    I[1][1] = factor * (wx * wx + wz * wz);
    I[2][2] = factor * (wx * wx + wy * wy);
    return I;
}
