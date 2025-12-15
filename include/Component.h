#pragma once
#include <string>

class Object; // forward declaration

class Component {
public:
    Object* owner = nullptr;
    virtual ~Component() {};

    virtual void start() {}
    virtual void update(float dt) {}
};