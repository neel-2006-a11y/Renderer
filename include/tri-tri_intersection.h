#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>

struct TriangleContact {
    glm::vec3 pointOnA;
    glm::vec3 pointOnB;

    glm::vec3 penetrationDir; // B-A
    float penetrationDepth = 0.0f;

    bool hit = false;
    bool isEdgeEdge = false;

    // debug
    bool isAVert = false;
    bool isBVert = false;

    glm::vec3 contactPoint;
};

TriangleContact computeTrianglePenetration(
    const glm::vec3& A0, const glm::vec3& A1, const glm::vec3& A2,
    const glm::vec3& B0, const glm::vec3& B1, const glm::vec3& B2);