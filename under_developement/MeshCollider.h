#pragma once
#include "Component.h"
#include "Object.h"
#include "bvh.h"
#include "transform.h"

// NOTE: still not gpu compatible due to BVH updating on CPU
class MeshCollider : public Component {
public:
    Model* model = nullptr;       // the mesh to collide with
    BVH* bvh = nullptr;           // built from the mesh
    AABB worldAABB;               // world-space bounding box

    bool isStatic = true;

    MeshCollider(Model* model = nullptr, bool isStatic = true)
        : model(model), isStatic(isStatic) {}

    void start() override {
        if (!model) {
            std::cerr << "[MeshCollider] No model assigned!\n";
            return;
        }

        // Build BVH once
        bvh = new BVH();
        if(isStatic){
            Transform* t = owner->transform;
            bvh->buildTransformed(*model, *t, 2);
            computeWorldAABB();
        }else{
            bvh->build(*model, 2);
        }
    }

    void update(float dt) override {
        if(!isStatic){
            Transform* t = owner->transform;
            bvh->refitTransformed(*t);
            computeWorldAABB();
        }
    }

    void computeWorldAABB() {
        if (!bvh) return;

        glm::mat4 M = owner->transform->getMatrix();
        worldAABB = bvh->root->box;
    }

    virtual ~MeshCollider() {
        if (bvh) {
            delete bvh;
            bvh = nullptr;
        }
    }
};
