// Object.h
#pragma once
#include <vector>
#include <type_traits>
#include "Component.h"
#include "transform.h"

class Object {
public:
    std::string name;
    Transform* transform;

    Object(const std::string& n = "Object") : name(n) {
        transform = addComponent<Transform>(); // every object has a Transform
    }

    ~Object() {
        for (auto c : components) delete c;
    }

    // Add component of type T
    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, 
            "T must derive from Component");

        T* c = new T(std::forward<Args>(args)...);
        c->owner = this;
        components.push_back(c);
        c->start();
        return c;
    }

    // Get first component of type T
    template<typename T>
    T* getComponent() {
        for (auto c : components) {
            if (auto ptr = dynamic_cast<T*>(c))
                return ptr;
        }
        return nullptr;
    }

    // Called every frame
    void update(float dt) {
        for (auto c : components)
            c->update(dt);
    }

private:
    std::vector<Component*> components;
};
