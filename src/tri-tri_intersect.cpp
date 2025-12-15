#include "tri-tri_intersection.h"


static inline float clamp(float x, float a, float b){
    return std::max(a,std::min(a,b));
}

static inline float distance2(const glm::vec3& a, const glm::vec3& b){
    return ((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y) + (b.z-a.z)*(b.z-a.z));
}

// closest point on segment ab to p
static inline glm::vec3 closestPointOnSegment(const glm::vec3& a, const glm::vec3& b, const glm::vec3 p)
{
    glm::vec3 ab = b-a;
    float t = glm::dot(p-a,ab) / glm::dot(ab,ab);
    t = clamp(t,0.0f,1.0f);
    return a + t * ab;
}


// closest point on triangle abc to p
static glm::vec3 closestPointOnTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    const glm::vec3 ab = b-a;
    const glm::vec3 ac = c-a;
    const glm::vec3 ap = p-a;

    // check vertex A
    float d1 = glm::dot(ab,ap);
    float d2 = glm::dot(ac,ap);
    if(d1<=0.0f && d2<=0.0f) return a;

    // Check vertex B
    const glm::vec3 bp = p-b;
    float d3 = glm::dot(ab,bp);
    float d4 = glm::dot(ac,bp);
    if(d3>=0.0f && d4 <= d3) return b;

    // Check edge AB
    float vc = d1*d4 - d3*d2;
    if(vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f){
        float v = d1/(d1-d3);
        return a+v*ab;
    }

    // Check vertex C
    const glm::vec3 cp = p - c;
    float d5 = glm::dot(ab, cp);
    float d6 = glm::dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c;

    // Check edge AC
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return a + w * ac;
    }

    // Check edge BC
    const glm::vec3 bc = c - b;
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * bc;
    }

    // Inside face region. Project to plane.
    // float denom = glm::dot(glm::cross(ab, ac), glm::cross(ab, ac));
    glm::vec3 n = glm::normalize(glm::cross(ab, ac));

    float dist = glm::dot(p - a, n);
    return p - dist * n;
}


static void closestPtSegmentSegment(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& p2, const glm::vec3& q2, glm::vec3& out1, glm::vec3& out2)
{
    glm::vec3 d1 = q1-p1;
    glm::vec3 d2 = q2-p2;
    glm::vec3 r = p1-p2;
    float a = glm::dot(d1,d1);
    float e = glm::dot(d2,d2);
    float f = glm::dot(d2,r);
    
    float s,t;

    if(a <= 1e-8f && e <= 1e-8f){
        out1 = p1;
        out2 = p2;
        return;
    }

    if(a <= 1e-8f){
        s = 0.0f;
        t = clamp(f/e, 0.0f, 1.0f);
    }else{
        float c = glm::dot(d1,r);
        if(e <= 1e-8f){
            t = 0.0f;
            s = clamp(-c/a, 0.0f,1.0f);
        }else{
            float b = glm::dot(d1,d2);
            float denom = a*e - b*b;

            if(denom!=0.0f){
                s = clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            }else{
                s = 0.0f;
            }

            t = (b * s + f) / e;

            if (t < 0.0f) {
                t = 0.0f;
                s = clamp(-c / a, 0.0f, 1.0f);
            }
            else if (t > 1.0f) {
                t = 1.0f;
                s = clamp((b - c) / a, 0.0f, 1.0f);
            }
        }
    }

    out1 = p1+d1*s;
    out2 = p2+d2*t;
}

TriangleContact computeTrianglePenetration(
    const glm::vec3& A0, const glm::vec3& A1, const glm::vec3& A2,
    const glm::vec3& B0, const glm::vec3& B1, const glm::vec3& B2)
{
    TriangleContact result;

    float bestDist2 = 1e30f;
    glm::vec3 bestA,bestB;

    // Vertex -> Triangle test
    auto testPointTriangle = [&](
        const glm::vec3& P,
        const glm::vec3& t0,
        const glm::vec3& t1,
        const glm::vec3& t2,
        bool isPfromA
    ){
        glm::vec3 cp = closestPointOnTriangle(P,t0,t1,t2);
        float d2 = distance2(cp,P);
        if(d2 < bestDist2){
            bestDist2 = d2;
            if(isPfromA){
                result.isAVert = true;
                result.isBVert = false;
                bestA = P;
                bestB = cp;
            }else{
                result.isAVert = false;
                result.isBVert = true;
                bestA = cp;
                bestB = P;
            }
        }
    };

    // Vertex->Triangle tests
    testPointTriangle(A0,B0,B1,B2,true);
    testPointTriangle(A1,B0,B1,B2,true);
    testPointTriangle(A2,B0,B1,B2,true);

    testPointTriangle(B0,A0,A1,A2,false);
    testPointTriangle(B1,A0,A1,A2,false);
    testPointTriangle(B2,A0,A1,A2,false);

    // Edge-Edge test
    auto testEdgePair = [&](
        const glm::vec3& a0, const glm::vec3 a1,
        const glm::vec3& b0, const glm::vec3 b1
    ){
        glm::vec3 pa,pb;
        closestPtSegmentSegment(a0,a1,b0,b1,pa,pb);
        float d2 = distance2(pa,pb);
        if(d2 < bestDist2){
            result.isAVert = false;
            result.isBVert = false;
            bestDist2 = d2;
            bestA = pa;
            bestB = pb;
            result.isEdgeEdge = true;
        }
    };

    // Edge-Edge tests
    testEdgePair(A0, A1, B0, B1);
    testEdgePair(A1, A2, B1, B2);
    testEdgePair(A2, A0, B2, B0);

    testEdgePair(A0, A1, B1, B2);
    testEdgePair(A1, A2, B2, B0);
    testEdgePair(A2, A0, B0, B1);

    testEdgePair(A0, A1, B2, B0);
    testEdgePair(A1, A2, B0, B1);
    testEdgePair(A2, A0, B1, B2);

    // Output result
    result.pointOnA = bestA;
    result.pointOnB = bestB;

    result.penetrationDir = bestB - bestA;
    result.penetrationDepth = glm::length(result.penetrationDir);

    result.contactPoint = (bestA + bestB) * 0.5f;

    return result;
}

static bool rayIntersectTriangle(const glm::vec3& orig, const glm::vec3& dir,
                                      const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                      float& t, float& u, float& v) {
    const float EPSILON = 1e-8f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(dir, edge2);
    float a = glm::dot(edge1, h);
    if (a > -EPSILON && a < EPSILON){
        // std::cout << "Ray is parallel to triangle\n";
        return false; // Ray is parallel to triangle
    }
    float f = 1.0f / a;
    glm::vec3 s = orig - v0;
    u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, edge1);
    v = f * glm::dot(dir, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    // Compute t to find out where the intersection point is on the line
    t = f * glm::dot(edge2, q);
    if (t > EPSILON) // Ray intersection
        return true;

    return false; // Line intersection but not a ray intersection
}

bool triTriIntersect(
    const glm::vec3& A0, const glm::vec3& A1, const glm::vec3& A2,
    const glm::vec3& B0, const glm::vec3& B1, const glm::vec3& B2)
{
    // std::cout << "Checking A: (" << A0.x << "," << A0.y << "," << A0.z << ") ("
    //           << A1.x << "," << A1.y << "," << A1.z << ") ("
    //           << A2.x << "," << A2.y << "," << A2.z << ")\n";
    // std::cout << "Against B: (" << B0.x << "," << B0.y << "," << B0.z << ") ("
    //           << B1.x << "," << B1.y << "," << B1.z << ") ("
    //           << B2.x << "," << B2.y << "," << B2.z << ")\n";
    float t, u, v;

    // Check all edges of A as segments against triangle B
    if (rayIntersectTriangle(A0, A1 - A0, B0, B1, B2, t, u, v) && t <= 1.0f) return true;
    if (rayIntersectTriangle(A1, A2 - A1, B0, B1, B2, t, u, v) && t <= 1.0f) return true;
    if (rayIntersectTriangle(A2, A0 - A2, B0, B1, B2, t, u, v) && t <= 1.0f) return true;

    // Check all edges of B as segments against triangle A
    if (rayIntersectTriangle(B0, B1 - B0, A0, A1, A2, t, u, v) && t <= 1.0f) return true;
    if (rayIntersectTriangle(B1, B2 - B1, A0, A1, A2, t, u, v) && t <= 1.0f) return true;
    if (rayIntersectTriangle(B2, B0 - B2, A0, A1, A2, t, u, v) && t <= 1.0f) return true;

    return false;
}