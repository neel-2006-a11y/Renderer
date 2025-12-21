#pragma once
#include "Component.h"
#include "AABB.h"

class Collider : public Component{
    public:
    virtual AABB getAABB() const = 0;
};