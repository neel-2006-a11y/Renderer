#pragma once
#include <glm/glm.hpp>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB();
    AABB(const glm::vec3& min_, const glm::vec3& max_);
    void expand(const AABB& other);
    bool intersects(const AABB& other) const;
};