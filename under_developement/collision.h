#pragma once
#include "bvh.h"
#include "rigidbody.h"
#include "tri-tri_intersection.h"

#include <glm/glm.hpp>
#include <iostream>


bool triTriIntersect(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                     const glm::vec3& u0, const glm::vec3& u1, const glm::vec3& u2);

// bool BVHIntersect(const BVHNode* nodeA, const BVHNode* nodeB, int depthA = 0, int depthB = 0);

struct CollisionContact{
    bool hit=false;
    glm::vec3 normal;  //A->B
    glm::vec3 position;
    float penetrationDepth;
};
bool getCollisionContact(BVH& bvhA,BVH& bvhB,CollisionContact& contact);

void solvePositionalCorrection(Rigidbody& A, Rigidbody& B,const CollisionContact& c);

void solveVelocityImpulse(
    Rigidbody& A,
    Rigidbody& B,
    const CollisionContact& c,
    float restitution,    // e.g., 0.0 = no bounce, 0.3 = rubber
    float friction        // e.g., 0.5
);