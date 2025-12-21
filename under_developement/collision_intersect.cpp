#include "collision.h"

// bool BVHIntersect(const BVHNode* a,const BVHNode* b, int depthA, int depthB)
// {
//     if (!a || !b) return false;
//     if (!a->box.intersects(b->box)){
//         std::cout << "No intersection at depth A: " << depthA << " B: " << depthB << "\n";
//         return false;
//     } 

//     // If both are leaves, check actual triangles
//     if (a->isLeaf && b->isLeaf) {
//         std::cout << "Checking triangle-triangle at depth A: " << depthA << " B: " << depthB << "\n";
//         for (const auto& ta : a->transformedTriangles) {
//             for (const auto& tb : b->transformedTriangles) {
//                 if (triTriIntersect(ta.v0, ta.v1, ta.v2,
//                                     tb.v0, tb.v1, tb.v2)) {
//                     return true;
//                 }
//             }
//         }
//         return false;
//     }

//     // Recurse down
//     if (a->isLeaf) {
//         return BVHIntersect(a, b->left, depthA, depthB+1) || BVHIntersect(a, b->right, depthA, depthB+1);
//     } else if (b->isLeaf) {
//         return BVHIntersect(a->left, b, depthA+1, depthB) || BVHIntersect(a->right, b,depthA+1, depthB);
//     } else {
//         return BVHIntersect(a->left, b->left,depthA+1, depthB+1) ||
//                BVHIntersect(a->left, b->right,depthA+1, depthB+1) ||
//                BVHIntersect(a->right, b->left, depthA+1, depthB+1) ||
//                BVHIntersect(a->right, b->right, depthA+1, depthB+1);
//     }
// }

////////////////////////////////////////////////////////////

struct ContactCandidate{
    BVHTriangle triA;
    BVHTriangle triB;
};

void collectIntersectingTriangles(
    BVHNode* a,
    BVHNode* b,
    std::vector<ContactCandidate>& out)
{
    if (!a->box.intersects(b->box))
        return;

    if (a->isLeaf && b->isLeaf)
    {
        for (const auto& ta : a->transformedTriangles)
            for (const auto& tb : b->transformedTriangles)
                out.push_back({ta, tb});
        return;
    }

    if(a->isLeaf){
        collectIntersectingTriangles(a,b->left,out);
        collectIntersectingTriangles(a,b->right,out);
    }else if(b->isLeaf){
        collectIntersectingTriangles(a->left,b,out);
        collectIntersectingTriangles(a->right,b,out);
    }else{
        collectIntersectingTriangles(a->left,b->left,out);
        collectIntersectingTriangles(a->left,b->right,out);
        collectIntersectingTriangles(a->right,b->left,out);
        collectIntersectingTriangles(a->right,b->right,out);
    }
}

bool getCollisionContact(
    BVH& bvhA,
    BVH& bvhB,
    CollisionContact& contact)
{
    std::vector<ContactCandidate> candidates;
    collectIntersectingTriangles(bvhA.root, bvhB.root, candidates);

    bool hit = false;
    float bestDepth = -FLT_MAX;
    
    //debug
    bool isAVert,isBVert;

    for (auto& c : candidates)
    {
        if(!triTriIntersect(c.triA.v0, c.triA.v1, c.triA.v2, c.triB.v0, c.triB.v1, c.triB.v2))
        {
            continue;
        }
        contact.hit = true;

        TriangleContact tc = computeTrianglePenetration(
            c.triA.v0, c.triA.v1, c.triA.v2,
            c.triB.v0, c.triB.v1, c.triB.v2
        );

        float depth = tc.penetrationDepth;

        if(depth>bestDepth){
            isAVert = tc.isAVert;
            isBVert = tc.isBVert;

            bestDepth = depth;
            hit = true;

            contact.position = tc.contactPoint;

            glm::vec3 finalNormal;
            if(tc.isEdgeEdge){
                finalNormal = glm::normalize(tc.penetrationDir);
            }
            else
            {
                glm::vec3 dir = glm::normalize(tc.penetrationDir);

                float dA = abs(glm::dot(c.triA.normal, dir));
                float dB = abs(glm::dot(c.triB.normal, -dir));

                if(dA>dB){
                    finalNormal = c.triA.normal;
                }else{
                    finalNormal = -c.triB.normal;
                }

                // if (glm::dot(finalNormal, tc.penetrationDir) < 0)
                //     finalNormal = -finalNormal;
            }
            contact.normal = glm::normalize(finalNormal);
        }
    }

    // if(hit){
    //     std::cout << "isAVert:" << isAVert << std::endl;
    //     std::cout << "isBVert:" << isBVert << std::endl;
    // }
    contact.penetrationDepth = bestDepth;
    return hit;
}