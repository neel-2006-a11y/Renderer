#pragma once
#include <glm/glm.hpp>
#include <cmath>

// projection radius of OBB onto axis
inline float obbProjectRadius(const glm::vec3 axis,
                              const glm::vec3 axes[3],
                              const glm::vec3 he)
{
    return std::fabs(glm::dot(axis, axes[0])) * he.x +
           std::fabs(glm::dot(axis, axes[1])) * he.y +
           std::fabs(glm::dot(axis, axes[2])) * he.z;
}

inline void getOBBFace(
    const glm::vec3& C,
    const glm::vec3 axes[3],
    const glm::vec3& HE,
    int axisIndex,
    float axisSign,
    glm::vec3& outNormal,
    glm::vec3 outVerts[4])
{
    outNormal = axes[axisIndex] * axisSign;

    glm::vec3 center = C + outNormal * HE[axisIndex];

    glm::vec3 u = axes[(axisIndex + 1) % 3] * HE[(axisIndex + 1) % 3];
    glm::vec3 v = axes[(axisIndex + 2) % 3] * HE[(axisIndex + 2) % 3];

    outVerts[0] = center + u + v;
    outVerts[1] = center + u - v;
    outVerts[2] = center - u - v;
    outVerts[3] = center - u + v;
}

void clipPolygonAgainstPlane(
    const std::vector<glm::vec3>& poly,
    const glm::vec3& planeN,
    float planeD,
    std::vector<glm::vec3>& out)
{
    out.clear();

    for (size_t i = 0; i < poly.size(); i++) {
        const glm::vec3& A = poly[i];
        const glm::vec3& B = poly[(i + 1) % poly.size()];

        float da = glm::dot(planeN, A) - planeD;
        float db = glm::dot(planeN, B) - planeD;

        if (da >= 0) out.push_back(A);

        if ((da >= 0) != (db >= 0)) {
            float t = da / (da - db);
            out.push_back(A + t * (B - A));
        }
    }
}


// edge-edge 

struct Edge{glm::vec3 a,b;};
static void getOBBEdges(
    const glm::vec3& C,
    const glm::vec3 axes[3],
    const glm::vec3& HE,
    int axisIdx,
    Edge outEdges[4]
){
    glm::vec3 A0 = axes[0] * HE.x;
    glm::vec3 A1 = axes[1] * HE.y;
    glm::vec3 A2 = axes[2] * HE.z;

    glm::vec3 h[3] = {A0, A1, A2};

    int u = axisIdx;
    int v = (axisIdx + 1) % 3;
    int w = (axisIdx + 2) % 3;

    outEdges[0].a = C + h[u] + h[v] + h[w];; outEdges[0].b = C - h[u] + h[v] + h[w];
    outEdges[1].a = C + h[u] + h[v] - h[w];; outEdges[1].b = C - h[u] + h[v] - h[w];
    outEdges[2].a = C + h[u] - h[v] - h[w];; outEdges[2].b = C - h[u] - h[v] - h[w];
    outEdges[3].a = C + h[u] - h[v] + h[w];; outEdges[3].b = C - h[u] - h[v] + h[w];
}

static void projectPointOntoAxis(
    const glm::vec3& point,
    const glm::vec3& axis,
    float& outProjection
){
    outProjection = glm::dot(point, axis);
}

static float projectionOverlap(float minA, float maxA, float minB, float maxB){
    return std::fmin(maxA, maxB) - std::fmax(minA, minB);
}

static void closestPtSegmentSegment(
    const glm::vec3& p1, const glm::vec3& q1,
    const glm::vec3& p2, const glm::vec3& q2,
    glm::vec3& outC1, glm::vec3& outC2
){
    glm::vec3 d1 = q1 - p1;
    glm::vec3 d2 = q2 - p2;
    glm::vec3 r  = p1 - p2;

    const float EPS = 1e-6f;

    float a = glm::dot(d1, d1);
    float e = glm::dot(d2, d2);

    // Handle degenerate segments
    if (a < EPS && e < EPS) {
        outC1 = p1;
        outC2 = p2;
        return;
    }
    if (a < EPS) {
        // S1 is a point
        float t = glm::clamp(glm::dot(p1 - p2, d2) / e, 0.0f, 1.0f);
        outC1 = p1;
        outC2 = p2 + t * d2;
        return;
    }
    if (e < EPS) {
        // S2 is a point
        float s = glm::clamp(glm::dot(p2 - p1, d1) / a, 0.0f, 1.0f);
        outC1 = p1 + s * d1;
        outC2 = p2;
        return;
    }

    // Check if segments are parallel
    // Note: if segments are parallel, this axis is the SAT selected axis
    // so we can optimize for that case
    float crossLen = glm::length(glm::cross(d1, d2));
    // std::cout << "Cross Length: " << crossLen << "\n";
    if (crossLen < EPS) {
        //debug
        // std::cout << "Segments are parallel\n";
        // Segments are parallel: project onto d1
        glm::vec3 axis = glm::normalize(d1);
        float p1s = glm::dot(p1, axis);
        float q1s = glm::dot(q1, axis);
        float p2s = glm::dot(p2, axis);
        float q2s = glm::dot(q2, axis);

        float s1 = glm::min(p1s, q1s);
        float e1 = glm::max(p1s, q1s);

        float s2 = glm::min(p2s, q2s);
        float e2 = glm::max(p2s, q2s);

        float overlapStart = glm::max(s1, s2);
        float overlapEnd   = glm::min(e1, e2);

        if (overlapStart <= overlapEnd) {
            // They overlap: choose midpoint of overlap
            float mid = 0.5f * (overlapStart + overlapEnd);

            // Convert midpoint scalar back to points on each segment
            auto pointOnSegment = [&](const glm::vec3& A, const glm::vec3& B, float midVal) {
                float As = glm::dot(A, axis);
                float Bs = glm::dot(B, axis);
                float t = (midVal - As) / (Bs - As);
                t = glm::clamp(t, 0.0f, 1.0f);
                return A + t * (B - A);
            };

            outC1 = pointOnSegment(p1, q1, mid);
            outC2 = pointOnSegment(p2, q2, mid);
            return;
        }

        // No overlap → fall through to normal closest endpoint case
    }

    // --- Non-parallel case: original algorithm ---
    float b = glm::dot(d1, d2);
    float c = glm::dot(d1, r);
    float f = glm::dot(d2, r);

    float denom = a * e - b * b;

    float s = 0.0f, t = 0.0f;

    if (denom != 0.0f)
        s = glm::clamp((b * f - c * e) / denom, 0.0f, 1.0f);

    t = (b * s + f) / e;

    if (t < 0.0f) {
        t = 0.0f;
        s = glm::clamp(-c / a, 0.0f, 1.0f);
    } else if (t > 1.0f) {
        t = 1.0f;
        s = glm::clamp((b - c) / a, 0.0f, 1.0f);
    }

    outC1 = p1 + s * d1;
    outC2 = p2 + t * d2;
}


bool computeOBBEdgeEdgeContact(
    const glm::vec3& centerA, const glm::vec3 axesA[3], const glm::vec3& HEA,
    const glm::vec3& centerB, const glm::vec3 axesB[3], const glm::vec3& HEB,
    int axisA, int axisB,
    const glm::vec3& penetrationAxis,
    glm::vec3& outPoint,
    glm::vec3& outNormal,
    float& outDepth
){
    Edge edgesA[4], edgesB[4];
    getOBBEdges(centerA, axesA, HEA, axisA, edgesA);
    getOBBEdges(centerB, axesB, HEB, axisB, edgesB);

    // find best edge-edge based on projection overlap
    float bestOverlap = FLT_MAX;
    int bestAidx = -1, bestBidx = -1;

    for (int i = 0; i < 4; i++)
    {
        float projA;
        projectPointOntoAxis(edgesA[i].a, penetrationAxis, projA);

        for (int j = 0; j < 4; j++)
        {
            float projB;
            projectPointOntoAxis(edgesB[j].a, penetrationAxis, projB);

            float overlap = abs(projA - projB);
            if (overlap < bestOverlap)
            {
                bestOverlap = overlap;
                bestAidx = i;
                bestBidx = j;
            }
        }
    }

    if (bestAidx < 0){
        // debug
        // std::cout << "No best edge-edge found!\n";
        return false; // no valid edge-edge contact
    }

    // Now compute closest points between the selected edges
    glm::vec3 pA, pB;
    closestPtSegmentSegment(
        edgesA[bestAidx].a, edgesA[bestAidx].b,
        edgesB[bestBidx].a, edgesB[bestBidx].b,
        pA, pB
    );
    // debug
    // std::cout << "Best Edge A: " << edgesA[bestAidx].a.x << "," << edgesA[bestAidx].a.y << "," << edgesA[bestAidx].a.z << " to "
    //           << edgesA[bestAidx].b.x << "," << edgesA[bestAidx].b.y << "," << edgesA[bestAidx].b.z << "\n";
    // std::cout << "Best Edge B: " << edgesB[bestBidx].a.x << "," << edgesB[bestBidx].a.y << "," << edgesB[bestBidx].a.z << " to "
    //           << edgesB[bestBidx].b.x << "," << edgesB[bestBidx].b.y << "," << edgesB[bestBidx].b.z << "\n";
    // std::cout << "Edge-Edge contact points: A(" << pA.x << "," << pA.y << "," << pA.z << ") B(" << pB.x << "," << pB.y << "," << pB.z << ")\n";
    outPoint = 0.5f * (pA + pB);
    outNormal = penetrationAxis; // SAT axis is correct normal
    outDepth = bestOverlap;

    return true;
}

struct OBBContact {
    bool hit = false;
    glm::vec3 normal; // box1 to box2
    float penetration;
    glm::vec3 contactPoint; // approximate contact point (world space)
};

struct Face{
    glm::vec3 normal;
    glm::vec3 verts[4];
};

void getIncidentFaces(
    const glm::vec3& C,
    const glm::vec3 axes[3],
    const glm::vec3& HE,
    const glm::vec3& referenceNormal,
    std::vector<Face*>& outFaces
) {
    outFaces.clear();
    
    for (int i = 0; i < 3; i++) {
        for (int sign = -1; sign <= 1; sign += 2) {
            glm::vec3 faceNormal = axes[i] * (float)sign;
            float dotProd = glm::dot(faceNormal, referenceNormal);
            if (dotProd < 0.0f) {
                Face* face = new Face();
                face->normal = faceNormal;
                getOBBFace(C, axes, HE, i, (float)sign, face->normal, face->verts);
                outFaces.push_back(face);
            }
        }
    }
}

glm::vec3 computeAveragePoint(const std::vector<glm::vec3>& points) {
    glm::vec3 avg(0.0f);
    for (const glm::vec3& p : points) {
        avg += p;
    }
    if (!points.empty()) {
        avg /= static_cast<float>(points.size());
    }
    return avg;
}


inline OBBContact testOBBvsOBB(
    const glm::vec3& C1, const glm::vec3 A1[3], const glm::vec3& HE1,
    const glm::vec3& C2, const glm::vec3 A2[3], const glm::vec3& HE2)
{
    OBBContact result;
    glm::vec3 d = C2 - C1;

    float bestPen = 1e9;
    glm::vec3 bestAxis;
    int bestAxisID = -1;
    int bestAxisSign = 1;

    auto testAxis = [&](const glm::vec3& axis, int id) {
        if (glm::length(axis) < 1e-8f) return true;

        glm::vec3 a = glm::normalize(axis);

        float r1 = obbProjectRadius(a, A1, HE1);
        float r2 = obbProjectRadius(a, A2, HE2);
        float dist = std::fabs(glm::dot(d, a));

        float overlap = r1 + r2 - dist;
        if (overlap < 0) return false;

        if (overlap < bestPen) {
            bestPen = overlap;
            bestAxis = a;
            bestAxisID = id;
            bestAxisSign = (glm::dot(d, a) < 0) ? -1 : 1;
        }
        return true;
    };

    // Test A1, A2, A3
    for(int i=0;i<3;i++) if(!testAxis(A1[i], i)) return result;

    // Test B1, B2, B3
    for(int i=0;i<3;i++) if(!testAxis(A2[i], i+3)) return result;

    // Test cross products
    int id = 6;
    for (int i=0;i<3;i++)
        for (int j=0;j<3;j++)
            if (!testAxis(glm::cross(A1[i], A2[j]), id++))
                return result;

    result.hit = true;
    result.penetration = bestPen;
    result.normal = bestAxis * (float)bestAxisSign;

    // approximate contact point
    glm::vec3 refC;
    glm::vec3 refAxes[3];
    glm::vec3 refHE;

    glm::vec3 incC;
    glm::vec3 incAxes[3];
    glm::vec3 incHE;

    int axisIdx = 0;
    int axisSign = bestAxisSign;
    // std::cout << "Best axis ID: " << bestAxisID << "\n";
    if(bestAxisID < 3){
        // Reference = BoxA
        refC = C1; refAxes[0] = A1[0]; refAxes[1] = A1[1]; refAxes[2] = A1[2]; refHE = HE1;
        incC = C2; incAxes[0] = A2[0]; incAxes[1] = A2[1]; incAxes[2] = A2[2]; incHE = HE2;
        axisIdx = bestAxisID;
    }else if(bestAxisID < 6){
        // Reference = BoxB
        refC = C2; refAxes[0] = A2[0]; refAxes[1] = A2[1]; refAxes[2] = A2[2]; refHE = HE2;
        incC = C1; incAxes[0] = A1[0]; incAxes[1] = A1[1]; incAxes[2] = A1[2]; incHE = HE1;
        axisIdx = bestAxisID - 3;
        axisSign = -bestAxisSign;
    }else{ // edge-edge collision
        computeOBBEdgeEdgeContact(
            C1, A1, HE1,
            C2, A2, HE2,
            (bestAxisID - 6) / 3,
            (bestAxisID - 6) % 3,
            bestAxis * (float)bestAxisSign,
            result.contactPoint,
            result.normal,
            result.penetration
        );
        return result;
    }

    glm::vec3 refNormal;
    glm::vec3 refFaceVerts[4];
    getOBBFace(refC, refAxes, refHE, axisIdx, axisSign, refNormal, refFaceVerts);
    // ensure CCW order
    if(glm::dot(glm::cross(refFaceVerts[1]-refFaceVerts[0], refFaceVerts[3]-refFaceVerts[0]), refNormal) < 0){
        std::swap(refFaceVerts[1], refFaceVerts[3]);
    }

    // New approach: use incident faces and clip them behind reference face plane
    std::vector<Face*> incFaces;
    getIncidentFaces(incC, incAxes, incHE, refNormal, incFaces);
    std::vector<glm::vec3> collectedVerts;
    for(const Face* f : incFaces){
        std::vector<glm::vec3> poly;
        for(int i=0;i<4;i++){
            poly.push_back(f->verts[i]);
        }
        std::vector<glm::vec3> tmp;
        for(int i=0;i<4;i++){
            glm::vec3 edge = refFaceVerts[(i+1)%4] - refFaceVerts[i];
            glm::vec3 planeN = glm::normalize(glm::cross(refNormal, edge));
            float planeD = glm::dot(planeN, refFaceVerts[i]);

            clipPolygonAgainstPlane(poly, planeN, planeD, tmp);
            poly = tmp;
            if(poly.empty()){
                // std::cout << "clipped away all vertices\n";
                break;
            }
        }
        // clip vertices behind reference face plane
        if(!poly.empty()){
            clipPolygonAgainstPlane(poly,-refNormal,glm::dot(-refNormal, refFaceVerts[0]),tmp);
            poly = tmp;
        }
        // collect remaining vertices
        for(const auto& v : poly){
            collectedVerts.push_back(v);
        }
        // average final clipped polygon vertices to get contact point
        result.contactPoint = computeAveragePoint(collectedVerts);
    }
    return result;
}

void resolveCubeCubeCollision(
    CubeCollider& boxA,
    CubeCollider& boxB,
    float restitution,
    bool debug=false
){
    glm::vec3 C1, C2;
    glm::vec3 A1[3], A2[3];
    glm::vec3 HE1;
    glm::vec3 HE2;
    AABB aabb1, aabb2;
    boxA.getAABBandOBB(aabb1,C1,HE1,A1);
    boxB.getAABBandOBB(aabb2,C2,HE2,A2);
    if(!aabb1.intersects(aabb2)) return;

    OBBContact contact = testOBBvsOBB(C1, A1, HE1, C2, A2, HE2);
    if(!contact.hit){return;}
    if(debug){
        std::cout << "OBB Collision detected!//////////////////////\n";
        std::cout << "collision position: (" << contact.contactPoint.x << ", " << contact.contactPoint.y << ", " << contact.contactPoint.z << ")\n";
    }

    // Contact point (midpoint)
    glm::vec3 contactPoint = contact.contactPoint;

    glm::vec3 ra = contactPoint - boxA.owner->transform->position;
    glm::vec3 rb = contactPoint - boxB.owner->transform->position;

    Rigidbody* rbA = boxA.owner->getComponent<Rigidbody>();
    Rigidbody* rbB = boxB.owner->getComponent<Rigidbody>();
    
    if(rbA && rbB){

        // Simple positional correction
        glm::vec3 correction = contact.normal * contact.penetration * 0.7f;

        // std::cout << "Applying positional correction: (" << correction.x << ", " << correction.y << ", " << correction.z << ")\n";  
        if(!rbA->isStatic)
            boxA.owner->transform->position -= correction;
        if(!rbB->isStatic)
            boxB.owner->transform->position += correction;
        
        ////////////////////////////////////////////////////////////
        // Velocities at contact
        glm::vec3 angularVelocityA = rbA->getInvInertiaTensorWorld() * rbA->angularMomentum;
        glm::vec3 angularVelocityB = rbB->getInvInertiaTensorWorld() * rbB->angularMomentum;
        glm::vec3 vA = rbA->velocity + glm::cross(angularVelocityA, ra);
        glm::vec3 vB = rbB->velocity + glm::cross(angularVelocityB, rb);

        glm::vec3 vRel = vB - vA;
        if(debug){
            std::cout << "Contact normal: (" << contact.normal.x << ", " << contact.normal.y << ", " << contact.normal.z << ")\n";
            std::cout << "Contact position: (" << contactPoint.x << ", " << contactPoint.y << ", " << contactPoint.z << ")\n";
            std::cout << "Relative velocity: (" << vRel.x << ", " << vRel.y << ", " << vRel.z << ")\n";
        }
        float vRelN = glm::dot(vRel, contact.normal);
        if (vRelN > 0.0f) return;

        glm::vec3 rnA = glm::cross(ra, contact.normal);
        glm::vec3 rnB = glm::cross(rb, contact.normal);

        float angularTerm = glm::dot(
            contact.normal,
            glm::cross(rbA->getInvInertiaTensorWorld() * rnA, ra) +
            glm::cross(rbB->getInvInertiaTensorWorld() * rnB, rb)
        );

        float denom = (rbA->invMass + rbB->invMass) + angularTerm;

        float j = -(1.0f + restitution) * vRelN / denom;

        glm::vec3 impulseN = j * contact.normal;
        if(debug)
            std::cout << "Normal impulse: (" << impulseN.x << ", " << impulseN.y << ", " << impulseN.z << ")\n";

        // Apply normal impulse
        // std::cout << "velocity after impulse:\n";
        rbA->applyImpulse(-impulseN, contact.contactPoint);
        // std::cout << "A: (" << rbA->velocity.x << ", " << rbA->velocity.y << ", " << rbA->velocity.z << ") ";
        rbB->applyImpulse( impulseN, contact.contactPoint);
        // std::cout << "B: (" << rbB->velocity.x << ", " << rbB->velocity.y << ", " << rbB->velocity.z << ") \n";
    }
}
