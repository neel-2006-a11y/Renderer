#pragma once
#include "Component.h"
#include "transform.h"
#include "Object.h"
#include "ModelHandle.h"

class MeshRenderer : public Component {
public:
    ModelHandle model;
    Shader* shader = nullptr;

    MeshRenderer(ModelHandle m, Shader* s) : model(m), shader(s) {}
};
