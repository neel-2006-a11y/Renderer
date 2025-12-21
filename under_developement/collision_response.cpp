#include "collision.h"

void solvePositionalCorrection(
    Rigidbody& A, 
    Rigidbody& B,
    const CollisionContact& c)
{
    if(!c.hit) return;
    const float k_slop = 0.001f;   // penetration tolerance
    const float k_percent = 0.8f;  // usually 0.6–0.8

    float depth = c.penetrationDepth - k_slop;
    if (depth < 0.0f) return;

    glm::vec3 correction = c.normal * depth * k_percent;

    float invMassSum = A.invMass + B.invMass;
    if (invMassSum <= 0.0f) return;

    // Move A backwards, B forwards
    if(!A.isStatic)
        A.owner->transform->position -= correction * (A.invMass / invMassSum);
    if(!B.isStatic)
        B.owner->transform->position += correction * (B.invMass / invMassSum);
}

void solveVelocityImpulse(
    Rigidbody& A,
    Rigidbody& B,
    const CollisionContact& c,
    float restitution,    // e.g., 0.0 = no bounce, 0.3 = rubber
    float friction        // e.g., 0.5
)
{
    if(!c.hit) return;
    //debug
    std::cout << "contact:\n";
    std::cout << "normal: ";
    glm::vec3 n = c.normal;
    std::cout << "(" << n.x << ", " << n.y << ", " << n.z << ")";

    std::cout << "position: ";
    std::cout << "(" << c.position.x << ", " << c.position.y << ", " << c.position.z << ")"; 

    std::cout << "penetration: ";
    std::cout << c.penetrationDepth << "\n";


    // Contact point (midpoint)
    glm::vec3 contactPoint = c.position;

    glm::vec3 ra = contactPoint - A.owner->transform->position;
    glm::vec3 rb = contactPoint - B.owner->transform->position;

    // Velocities at contact
    glm::vec3 angularVelocityA = A.getInvInertiaTensorWorld() * A.angularMomentum;
    glm::vec3 angularVelocityB = B.getInvInertiaTensorWorld() * B.angularMomentum;
    glm::vec3 vA = A.velocity + glm::cross(angularVelocityA, ra);
    glm::vec3 vB = B.velocity + glm::cross(angularVelocityB, rb);

    glm::vec3 vRel = vB - vA;
    float vRelN = glm::dot(vRel, n);
    std::cout << "vRelN: " << vRelN << "\n";

    // If separating, no impulse needed
    if (vRelN > 0.0f) return;

    // Effective mass in normal direction
    // float denom =
    //     (A.invMass + B.invMass) +
    //     glm::dot(n,
    //         glm::cross(A.invInertiaTensor * glm::cross(ra, n), ra)
    //       + glm::cross(B.invInertiaTensor * glm::cross(rb, n), rb)
    //     );
    float denom = (A.invMass + B.invMass);

    float j = -(1.0f + restitution) * vRelN / denom;

    glm::vec3 impulseN = j * n;

    // Apply normal impulse
    std::cout << "velocity after impulse:\n";
    A.applyImpulse(-impulseN, ra);
    std::cout << "A: (" << A.velocity.x << ", " << A.velocity.y << ", " << A.velocity.z << ") ";
    B.applyImpulse( impulseN, rb);
    std::cout << "B: (" << B.velocity.x << ", " << B.velocity.y << ", " << B.velocity.z << ") \n";
    
    //debug
    // std::cout <<"FLAG1\n";
    return;
}
