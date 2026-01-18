#include "OBBCollision.h"

// ===================== Math helpers =====================
float obbProjectRadius(
    const glm::vec3 axis,
    const glm::vec3 axes[3],
    const glm::vec3 he
){
    return std::fabs(glm::dot(axis, axes[0])) * he.x +
           std::fabs(glm::dot(axis, axes[1])) * he.y +
           std::fabs(glm::dot(axis, axes[2])) * he.z;
}

void getOBBFace(
    const glm::vec3& C,
    const glm::vec3 axes[3],
    const glm::vec3& HE,
    int axisIndex,
    float axisSign,
    glm::vec3& outNormal,
    glm::vec3 outVerts[4]
){
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
    std::vector<glm::vec3>& out
){
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

// =================== Edge Helpers ================

void getOBBEdges(
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

void closestPtSegmentSegment(
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

static void projectPointOntoAxis(
    const glm::vec3& point,
    const glm::vec3& axis,
    float& outProjection
){
    outProjection = glm::dot(point, axis);
}


// ====================== Manifold Helpers ===========

bool isPointUnique(std::vector<glm::vec3>& verts, glm::vec3& p, float eps){
    for(auto q:verts){
        if(glm::dot(p-q,p-q) < eps){
            return false;
        }
    }
    return true;
} 

PlaneBasis buildPlaneBasis(const glm::vec3& n){
    glm::vec3 t = (std::abs(n.x) > 0.9f)
        ? glm::vec3(0,1,0)
        : glm::vec3(1,0,0);

    PlaneBasis b;
    b.u = glm::normalize(glm::cross(n,t));
    b.v = glm::cross(n,b.u);
    return b;
}

glm::vec2 projectToPlane(
    const glm::vec3& p,
    const glm::vec3& origin,
    const PlaneBasis& b
){
    glm::vec3 d = p - origin;
    return{
        glm::dot(d,b.u),
        glm::dot(d,b.v)
    };
}

void reduceManifold(
    const std::vector<glm::vec3>& inPts,
    const glm::vec3& normal,
    float penetration,
    std::vector<ContactPoint>& out
){
    out.clear();
    if(inPts.empty()) return;

    // 1) Find deepest point
    int deepest = 0;
    float minProj = FLT_MAX;
    for (int i = 0; i < (int)inPts.size(); i++) {
        float d = glm::dot(inPts[i], normal);
        if (d < minProj) {
            minProj = d;
            deepest = i;
        }
    }

    glm::vec3 origin = inPts[deepest];
    PlaneBasis basis = buildPlaneBasis(normal);

    // project to 2d
    std::vector<glm::vec2> pts2D;

    for(auto& p : inPts)
        pts2D.push_back(projectToPlane(p,origin,basis));

    // 2) pick the furthest from deepest

    int i1 = deepest;
    int i2 = i1;
    float maxDist = 0.0f;
    
    for (int i = 0; i < (int)pts2D.size(); i++) {
        float d = glm::dot((pts2D[i] - pts2D[i1]),(pts2D[i] - pts2D[i1]));
        if (d > maxDist) {
            maxDist = d;
            i2 = i;
        }
    }

    // 3. pick points that maimize triangle area
    int i3 = i1, i4 = i1;
    float maxArea1 = 0.0f, maxArea2 = 0.0f;

    glm::vec2 a = pts2D[i1];
    glm::vec2 b = pts2D[i2];

    for (int i = 0; i < (int)pts2D.size(); i++) {
        float area = std::abs(glm::cross(
            glm::vec3(b - a, 0),
            glm::vec3(pts2D[i] - a, 0)
        ).z);

        if (area > maxArea1) {
            maxArea2 = maxArea1; i4 = i3;
            maxArea1 = area;     i3 = i;
        } else if (area > maxArea2) {
            maxArea2 = area;     i4 = i;
        }
    }

    // Collect unique indices
    std::vector<int> indices = { i1, i2, i3, i4 };
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    for (int idx : indices) {
        out.push_back({
            inPts[idx],
            normal,
            penetration
        });
        if (out.size() == 4) break;
    }
}

// ===================== Collision Coroutines ===========

bool computeOBBEdgeEdgeContact(
    const glm::vec3& centerA, const glm::vec3 axesA[3], const glm::vec3& HEA,
    const glm::vec3& centerB, const glm::vec3 axesB[3], const glm::vec3& HEB,
    int axisA, int axisB,
    const glm::vec3& penetrationAxis,
    ContactManifold& outManifold,
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

    outNormal = penetrationAxis; // SAT axis is correct normal
    outDepth = bestOverlap;
    outManifold.points.push_back({0.5f * (pA + pB), outNormal, outDepth, 0.0f});
    return true;
}

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
            result.manifold,
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
        for(auto& v : poly){
            collectedVerts.push_back(v);
        }
    }
    // unique vertices
    std::vector<glm::vec3> uniqueVerts;
    for(glm::vec3& v : collectedVerts) {
        if(isPointUnique(uniqueVerts, v)){
            uniqueVerts.push_back(v);
        }
    }

    // contact manifold building
    reduceManifold(uniqueVerts, result.normal, result.penetration, result.manifold.points);
    return result;
}

//================= Resolve Collision ==================
void resolveCubeCubeCollision(
    CubeCollider& boxA,
    CubeCollider& boxB,
    float restitution,
    int solveIterations
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
    
    Rigidbody* rbA = boxA.owner->getComponent<Rigidbody>();
    Rigidbody* rbB = boxB.owner->getComponent<Rigidbody>();
    if(!rbA || !rbB) return;
    // ======= position correction =====
    glm::vec3 correction = contact.normal * contact.penetration;
    float extent = 0.7;
    if(rbA->isStatic || rbB->isStatic) extent = 1.0f;
    
    if(!rbA->isStatic)
        rbA->transform->position -= correction*extent;
    if(!rbB->isStatic)
        rbB->transform->position += correction*extent;
    for(int iter = 0; iter < solveIterations; iter++){
        for(ContactPoint& c : contact.manifold.points){

            glm::vec3 n = c.normal;
            // ======= impulse response ====
            // ======= normal impulse ======
            glm::vec3 ra = c.position - boxA.owner->transform->position;
            glm::vec3 rb = c.position - boxB.owner->transform->position;

            glm::vec3 wA = rbA->getInvInertiaTensorWorld() * rbA->angularMomentum;
            glm::vec3 wB = rbB->getInvInertiaTensorWorld() * rbB->angularMomentum;

            glm::vec3 vA = rbA->velocity + glm::cross(wA, ra);
            glm::vec3 vB = rbB->velocity + glm::cross(wB, rb);

            glm::vec3 vRel = vB-vA;
            // std::cout << "vRel^2: " << glm::dot(vRel,vRel) << "\n";
            float vRelN = glm::dot(vRel,n);

            float epsV = 0.01f;
            if(vRelN>epsV){ // seperating
                continue;
            }

            float e = (iter == 0) ? restitution : 0.0f;

            glm::vec3 rnA = glm::cross(ra,n);
            glm::vec3 rnB = glm::cross(rb,n);

            float angularTerm = 
                glm::dot(
                        n,
                        glm::cross(rbA->getInvInertiaTensorWorld() * rnA, ra) +
                        glm::cross(rbB->getInvInertiaTensorWorld() * rnB, rb)
                    );
            float denom = rbA->invMass + rbB->invMass + angularTerm;
            if(denom < 1e-6f) continue;

            float j = -(1.0f + e) * vRelN / denom;
            // std::cout << "vRelN: " << vRelN << "\n";
            // std::cout << "denom: " << denom << "\n";

            // Accumulate & clamp
            float oldImpulse = c.normalImpulse;
            c.normalImpulse = glm::max(oldImpulse + j, 0.0f);
            float deltaJ = c.normalImpulse - oldImpulse;

            glm::vec3 impulseN = deltaJ * n;

            if(!rbA->isStatic)
                rbA->applyImpulse(-impulseN, c.position);
            if(!rbB->isStatic)
                rbB->applyImpulse(impulseN, c.position);
        }
    }

    // ======= friction =====
    for(int iter = 0; iter < solveIterations; iter++){
        for(ContactPoint& c: contact.manifold.points){
            glm::vec3 n = c.normal;

            glm::vec3 ra = c.position - boxA.owner->transform->position;
            glm::vec3 rb = c.position - boxB.owner->transform->position;

            glm::vec3 wA = rbA->getInvInertiaTensorWorld() * rbA->angularMomentum;
            glm::vec3 wB = rbB->getInvInertiaTensorWorld() * rbB->angularMomentum;

            glm::vec3 vA = rbA->velocity + glm::cross(wA, ra);
            glm::vec3 vB = rbB->velocity + glm::cross(wB, rb);

            glm::vec3 vRel = vB-vA;

            float vRelN = glm::dot(vRel,n);

            glm::vec3 vRelT = vRel - n * vRelN;
            float vtLen2 = glm::dot(vRelT,vRelT);
            float EPS = 1e-4f;
            // float EPS = 0.0f;
            if(vtLen2>EPS){
                glm::vec3 t = vRelT / sqrt(vtLen2); // stable normalize

                // --- recompute angular terms for tangent ---
                glm::vec3 rtA = glm::cross(ra, t);
                glm::vec3 rtB = glm::cross(rb, t);

                float tangentDenom =
                    rbA->invMass + rbB->invMass +
                    glm::dot(
                        t,
                        glm::cross(rbA->getInvInertiaTensorWorld() * rtA, ra) +
                        glm::cross(rbB->getInvInertiaTensorWorld() * rtB, rb)
                    );

                if (tangentDenom > 1e-6f)
                {
                    float jt = -glm::dot(vRel, t) / tangentDenom;

                    float frictionCoeff = 0.1f;
                    float maxFriction = frictionCoeff * c.normalImpulse;

                    float oldT = c.tangentImpulse;
                    c.tangentImpulse = glm::clamp(oldT + jt, -maxFriction, maxFriction);
                    float deltaT = c.tangentImpulse - oldT;

                    glm::vec3 impulse = deltaT * t;

                    if (!rbA->isStatic)
                        rbA->applyImpulse(-impulse, c.position);
                    if (!rbB->isStatic)
                        rbB->applyImpulse( impulse, c.position);
                }
            } else {
                // ===== static friction stabilization =====

                // Only do this if contact is active (supporting)
                if (c.normalImpulse > 0.0f)
                {
                    // Tangential relative velocity is already tiny
                    // Kill it explicitly (Baumgarte-style velocity fix)

                    glm::vec3 vT = vRelT;  // already very small

                    // Compute effective mass along tangent
                    glm::vec3 t = glm::normalize(vT + glm::vec3(1e-8f));

                    glm::vec3 rtA = glm::cross(ra, t);
                    glm::vec3 rtB = glm::cross(rb, t);

                    float denom =
                        rbA->invMass + rbB->invMass +
                        glm::dot(
                            t,
                            glm::cross(rbA->getInvInertiaTensorWorld() * rtA, ra) +
                            glm::cross(rbB->getInvInertiaTensorWorld() * rtB, rb)
                        );

                    if (denom > 1e-6f)
                    {
                        float jt = -glm::dot(vRel, t) / denom;

                        glm::vec3 impulse = jt * t;

                        if (!rbA->isStatic)
                            rbA->applyImpulse(-impulse, c.position);
                        if (!rbB->isStatic)
                            rbB->applyImpulse( impulse, c.position);
                    }
                }
            }
        }
    }
}

