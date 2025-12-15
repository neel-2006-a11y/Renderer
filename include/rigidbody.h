#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include "Object.h"
#include "transform.h"

class Rigidbody : public Component {
public:
    // state
    Transform* transform; // cached pointer to owner's transform

    glm::vec3 velocity{0};
    glm::vec3 angularMomentum{0};

    float mass = 1.0f;
    float invMass = 1.0f;

    glm::mat3 inertiaTensorLocal = glm::mat3(1.0f); // local space
    glm::mat3 invInertiaTensorLocal = glm::mat3(1.0f); // local space

    bool isStatic = false;
    bool useGravity = false;
    glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);

    Rigidbody() {}
    Rigidbody(const Rigidbody& rb){
        this->velocity = rb.velocity;
        this->angularMomentum = rb.angularMomentum;
        this->mass = rb.mass;
        this->isStatic = rb.isStatic;
    }

    void start() override {
        transform = owner->transform;

        if(mass > 0.0f){
            invMass = 1.0f / mass;
            invInertiaTensorLocal = glm::inverse(inertiaTensorLocal);
        } else {
            invMass = 0.0f;
            isStatic = true;
        }
    }

    void update(float dt) override {
        if (isStatic) return;
        if (useGravity) {
            velocity += gravity * dt;
        }
        transform->position += velocity * dt;

        glm::vec3 angularVelocity = getInvInertiaTensorWorld() * angularMomentum;
        glm::quat deltaRotation = glm::quat(0, angularVelocity * 0.5f * dt);
        transform->rotation += deltaRotation * transform->rotation;
        transform->rotation = glm::normalize(transform->rotation);
    }


    void applyImpulse(const glm::vec3& impulse, const glm::vec3& contactPoint) {
        if (isStatic) return;

        velocity += impulse * invMass;

        glm::vec3 r = contactPoint - transform->position;
        glm::mat3 invInertiaWorld = getInvInertiaTensorWorld();

        glm::vec3 deltaAngularMomentum = glm::cross(r, impulse);
        // std::cout << "Delta Angular Momentum: (" << deltaAngularMomentum.x << ", " << deltaAngularMomentum.y << ", " << deltaAngularMomentum.z << ")\n";
        angularMomentum += deltaAngularMomentum;
    }

    glm::mat3 getInvInertiaTensorWorld() const {
        glm::mat3 R = glm::mat3_cast(transform->rotation);
        return R * invInertiaTensorLocal * glm::transpose(R);
    }
};
