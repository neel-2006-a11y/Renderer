#include "AABB.h"
void AABB::expand(const AABB& other) {
    min = glm::min(min, other.min);
    max = glm::max(max, other.max);
}

bool AABB::intersects(const AABB& other) const {
    const float eps = 1e-5f;
    return (min.x <= other.max.x + eps && max.x >= other.min.x - eps) &&
           (min.y <= other.max.y + eps && max.y >= other.min.y - eps) &&
           (min.z <= other.max.z + eps && max.z >= other.min.z - eps);
}
AABB::AABB(){
    min = glm::vec3(FLT_MAX);
    max = glm::vec3(-FLT_MAX);
}

AABB::AABB(const glm::vec3& min_, const glm::vec3& max_)
: min(min_), max(max_) {}