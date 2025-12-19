#pragma once

#include "CubeCollider.h"
#include "AABB.h"
#include "rigidbody.h"
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <cfloat>


// ===================== Basic helpers =====================
float obbProjectRadius(
    const glm::vec3 axis,
    const glm::vec3 axes[3],
    const glm::vec3 he
);

void getOBBFace(
    const glm::vec3& C,
    const glm::vec3 axes[3],
    const glm::vec3& HE,
    int axisIndex,
    float axisSign,
    glm::vec3& outNormal,
    glm::vec3 outVerts[4]
);

void clipPolygonAgainstPlane(
    const std::vector<glm::vec3>& poly,
    const glm::vec3& planeN,
    float planeD,
    std::vector<glm::vec3>& out
);

// ===================== Edge helpers =====================
struct Edge { glm::vec3 a, b; };

void getOBBEdges(
    const glm::vec3& C,
    const glm::vec3 axes[3],
    const glm::vec3& HE,
    int axisIdx,
    Edge outEdges[4]
);

void closestPtSegmentSegment(
    const glm::vec3& p1, const glm::vec3& q1,
    const glm::vec3& p2, const glm::vec3& q2,
    glm::vec3& outC1, glm::vec3& outC2
);

static void projectPointOntoAxis(
    const glm::vec3& point,
    const glm::vec3& axis,
    float& outProjection
);

// ===================== Contact structures =====================
struct ContactPoint {
    glm::vec3 position;
    glm::vec3 normal;
    float penetration;
    float normalImpulse = 0.0f;
    float tangentImpulse = 0.0f;
};

struct ContactManifold {
    std::vector<ContactPoint> points;
};

struct OBBContact {
    bool hit = false;
    glm::vec3 normal;
    float penetration;
    ContactManifold manifold;
};

struct Face {
    glm::vec3 normal;
    glm::vec3 verts[4];
};

// ===================== Manifold helpers =====================
bool isPointUnique(
    std::vector<glm::vec3>& verts,
    glm::vec3& p,
    float eps = 0.0001f
);

struct PlaneBasis {
    glm::vec3 u, v;
};

PlaneBasis buildPlaneBasis(const glm::vec3& n);

glm::vec2 projectToPlane(
    const glm::vec3& p,
    const glm::vec3& origin,
    const PlaneBasis& b
);

void reduceManifold(
    const std::vector<glm::vec3>& inPts,
    const glm::vec3& normal,
    float penetration,
    std::vector<ContactPoint>& out
);

// ===================== Collision routines =====================
bool computeOBBEdgeEdgeContact(
    const glm::vec3& centerA, const glm::vec3 axesA[3], const glm::vec3& HEA,
    const glm::vec3& centerB, const glm::vec3 axesB[3], const glm::vec3& HEB,
    int axisA, int axisB,
    const glm::vec3& penetrationAxis,
    ContactManifold& outManifold,
    glm::vec3& outNormal,
    float& outDepth
);

void getIncidentFaces(
    const glm::vec3& C,
    const glm::vec3 axes[3],
    const glm::vec3& HE,
    const glm::vec3& referenceNormal,
    std::vector<Face*>& outFaces
);

OBBContact testOBBvsOBB(
    const glm::vec3& C1, const glm::vec3 A1[3], const glm::vec3& HE1,
    const glm::vec3& C2, const glm::vec3 A2[3], const glm::vec3& HE2
);

void resolveCubeCubeCollision(
    CubeCollider& boxA,
    CubeCollider& boxB,
    float restitution,
    int solveIterations = 10
);
