#include "CubeCollider.h"

void CubeCollider::getOBB(glm::vec3& outCenter, glm::vec3& outHE, glm::vec3 outAxis[3]) const {
    outCenter = owner->transform->position;

    const Transform& T = *(owner->transform);
    glm::mat4 M = T.getMatrix();
    outAxis[0] = glm::vec3(M[0][0], M[0][1], M[0][2]);
    outAxis[1] = glm::vec3(M[1][0], M[1][1], M[1][2]);
    outAxis[2] = glm::vec3(M[2][0], M[2][1], M[2][2]);

    outHE = halfExtents;
}

AABB CubeCollider::getAABB() const {
    glm::vec3 C;
    glm::vec3 axes[3];
    glm::vec3 HE;
    getOBB(C, HE, axes);

    glm::vec3 absAxes[3] = {
        glm::vec3(fabs(axes[0].x), fabs(axes[0].y), fabs(axes[0].z)),
        glm::vec3(fabs(axes[1].x), fabs(axes[1].y), fabs(axes[1].z)),
        glm::vec3(fabs(axes[2].x), fabs(axes[2].y), fabs(axes[2].z))
    };

    glm::vec3 extents = 
        absAxes[0] * HE.x +
        absAxes[1] * HE.y +
        absAxes[2] * HE.z;

    return AABB(C - extents, C + extents);
}

void CubeCollider::getAABBandOBB(AABB& outAABB, glm::vec3& outCenter, glm::vec3& outHE, glm::vec3 outAxis[3]) const {
    getOBB(outCenter, outHE, outAxis);

    glm::vec3 absAxes[3] = {
        glm::vec3(fabs(outAxis[0].x), fabs(outAxis[0].y), fabs(outAxis[0].z)),
        glm::vec3(fabs(outAxis[1].x), fabs(outAxis[1].y), fabs(outAxis[1].z)),
        glm::vec3(fabs(outAxis[2].x), fabs(outAxis[2].y), fabs(outAxis[2].z))
    };

    glm::vec3 extents = 
        absAxes[0] * outHE.x +
        absAxes[1] * outHE.y +
        absAxes[2] * outHE.z;

    outAABB.min = outCenter - extents;
    outAABB.max = outCenter + extents;
}
