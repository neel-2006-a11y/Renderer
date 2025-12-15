#pragma once
#include "Component.h"
#include "transform.h"
#include "Object.h"
#include "model.h" // your model/mesh system

class MeshRenderer : public Component {
public:
    Model* model = nullptr;
    Shader* shader = nullptr;

    MeshRenderer(Model* m, Shader* s) : model(m), shader(s) {}
    
    void update(float dt) override {
        drawModel(*model, owner->transform->getMatrix(), shader);
    }
};
