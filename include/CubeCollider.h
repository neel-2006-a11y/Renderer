#include "transform.h"
#include "AABB.h"
#include "Collider.h"
#include "Object.h"

class CubeCollider : public Collider {
    public:
    glm::vec3 halfExtents;

    CubeCollider(glm::vec3 size = glm::vec3(1.0)) : halfExtents(0.5f*size) {}

    void getOBB(glm::vec3& outCenter, glm::vec3& outHE, glm::vec3 outAxis[3]) const;

    AABB getAABB() const override;

    void getAABBandOBB(AABB& outAABB, glm::vec3& outCenter, glm::vec3& outHE, glm::vec3 outAxis[3]) const;
};