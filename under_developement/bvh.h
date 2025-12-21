#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <limits>
#include "mesh.h"
#include "transform.h"
#include "AABB.h"

struct BVHTriangle {
    glm::vec3 v0, v1, v2;
    glm::vec3 normal;
    int id;
};

struct BVHNode {
    AABB box;
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    std::vector<BVHTriangle> baseTriangles;
    std::vector<BVHTriangle> transformedTriangles;
    bool isLeaf = false;
};

class BVH {
public:
    BVHNode* root = nullptr;

    BVH() = default;
    ~BVH();

    void build(const Model& model, int maxLeafSize = 2);
    void build(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, int maxLeafSize = 2);

    void buildTransformed(const Model& model, const Transform& transform, int maxLeafSize = 2);
    void buildTransformed(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const Transform& transform, int maxLeafSize = 2);

    void refitTransformed(const Transform& transform);
    
    bool intersectAABB(const AABB& box) const;
    void clear();

private:
    BVHNode* buildRecursiveTranformed(std::vector<BVHTriangle>& tris, int start, int end, int maxLeafSize);
    BVHNode* buildRecursive(std::vector<BVHTriangle>& tris, int start, int end, int maxLeafSize);
    void destroyRecursive(BVHNode* node);
};