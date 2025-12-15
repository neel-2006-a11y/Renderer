#include <bvh.h>
#include <cfloat>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// ---------------- BVH Implementation ----------------
static AABB computeAABB(const BVHTriangle& tri) {
    AABB box;
    box.min = glm::min(glm::min(tri.v0, tri.v1), tri.v2);
    box.max = glm::max(glm::max(tri.v0, tri.v1), tri.v2);
    return box;
}

static AABB mergeAABB(const AABB& a, const AABB& b) {
    AABB result;
    result.min = glm::min(a.min, b.min);
    result.max = glm::max(a.max, b.max);
    return result;
}

void BVH::build(const Model& model, int maxLeafSize) {
    std::vector<BVHTriangle> tris;
    for(const auto& mesh : model.meshes){
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            BVHTriangle tri;
            tri.v0 = mesh.vertices[mesh.indices[i]].position;
            tri.v1 = mesh.vertices[mesh.indices[i + 1]].position;
            tri.v2 = mesh.vertices[mesh.indices[i + 2]].position;

            tri.normal = mesh.vertices[mesh.indices[i]].normal+mesh.vertices[mesh.indices[i+1]].normal+mesh.vertices[mesh.indices[i+2]].normal;
            tri.normal = normalize(tri.normal);

            tri.id = static_cast<int>(i / 3);
            tris.push_back(tri);
        }
    }

    root = buildRecursive(tris, 0, (int)tris.size(), maxLeafSize);
}
void BVH::build(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, int maxLeafSize) {
    if (indices.empty()) return;

    std::vector<BVHTriangle> tris;
    for (size_t i = 0; i < indices.size(); i += 3) {
        BVHTriangle tri;
        tri.v0 = vertices[indices[i]].position;
        tri.v1 = vertices[indices[i + 1]].position;
        tri.v2 = vertices[indices[i + 2]].position;

        tri.normal = vertices[indices[i]].normal+vertices[indices[i+1]].normal+vertices[indices[i+2]].normal;
        tri.normal = normalize(tri.normal);

        tri.id = static_cast<int>(i / 3);
        tris.push_back(tri);
    }

    root = buildRecursive(tris, 0, (int)tris.size(), maxLeafSize);
}


void BVH::buildTransformed(const Model& model, const Transform& transform, int maxLeafSize) {
    clear();

    glm::mat4 modelMat = transform.getMatrix();
    size_t totalVertices = 0;
    size_t totalTris = 0;
    for(const auto& mesh : model.meshes){
        totalVertices += mesh.vertices.size();
        totalTris += mesh.indices.size() / 3;
    }
    std::vector<glm::vec3> transformedPositions;
    std::vector<glm::vec3> transformedNormals;
    transformedPositions.reserve(totalVertices);
    transformedNormals.reserve(totalVertices);

    for(const auto& mesh : model.meshes){
        for (const auto& vert : mesh.vertices) {
            glm::vec4 transformedPos = modelMat * glm::vec4(vert.position, 1.0f);
            transformedPositions.push_back(glm::vec3(transformedPos));

            glm::vec3 transformedNorm = normalize(glm::transpose(glm::inverse(glm::mat3(modelMat)))*vert.normal);
            transformedNormals.push_back(transformedNorm);
        }
    }
    std::vector<BVHTriangle> tris;
    tris.reserve(totalTris);
    size_t vertexOffset = 0;
    for(const auto& mesh : model.meshes){
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            BVHTriangle tri;
            tri.v0 = transformedPositions[vertexOffset + mesh.indices[i]];
            tri.v1 = transformedPositions[vertexOffset + mesh.indices[i + 1]];
            tri.v2 = transformedPositions[vertexOffset + mesh.indices[i + 2]];

            tri.normal = transformedNormals[vertexOffset + mesh.indices[i]]+
                         transformedNormals[vertexOffset + mesh.indices[i+1]]+
                         transformedNormals[vertexOffset + mesh.indices[i+2]];
            tri.normal = normalize(tri.normal);
            tri.id = static_cast<int>(i / 3);
            tris.push_back(tri);
        }
    }
    root = buildRecursiveTranformed(tris, 0, tris.size(), maxLeafSize);
}

void BVH::buildTransformed(
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices,
    const Transform& transform,
    int maxLeafSize
) {
    clear();

    // --- Build model matrix from Transform ---
    glm::mat4 model = transform.getMatrix();

    // --- Apply transformation to vertex positions ---
    std::vector<glm::vec3> transformedPositions;
    std::vector<glm::vec3> transformedNormals;
    transformedPositions.reserve(vertices.size());
    transformedNormals.reserve(vertices.size());

    for (const auto& vert : vertices) {
        glm::vec4 transformedPos = model * glm::vec4(vert.position, 1.0f);
        transformedPositions.push_back(glm::vec3(transformedPos));

        glm::vec3 transformedNorm = normalize(glm::transpose(glm::inverse(glm::mat3(model)))*vert.normal);
        transformedNormals.push_back(transformedNorm);
    }

    // --- Create triangle list ---
    std::vector<BVHTriangle> tris;
    tris.reserve(indices.size() / 3);
    for (size_t i = 0; i < indices.size(); i += 3) {
        BVHTriangle tri;
        tri.v0 = transformedPositions[indices[i]];
        tri.v1 = transformedPositions[indices[i + 1]];
        tri.v2 = transformedPositions[indices[i + 2]];

        tri.normal = transformedNormals[indices[i]]+transformedNormals[indices[i+1]]+transformedNormals[indices[i+2]];
        tri.normal = normalize(tri.normal);

        tri.id = static_cast<int>(i / 3);
        tris.push_back(tri);
    }

    // --- Build BVH recursively ---
    root = buildRecursiveTranformed(tris, 0, tris.size(), maxLeafSize);
}

BVHNode* BVH::buildRecursiveTranformed(std::vector<BVHTriangle>& tris, int start, int end, int maxLeafSize) {
    BVHNode* node = new BVHNode();

    AABB bounds;
    for (int i = start; i < end; ++i)
        bounds.expand(computeAABB(tris[i]));
    node->box = bounds;

    int count = end - start;
    if (count <= maxLeafSize) {
        node->isLeaf = true;
        node->transformedTriangles.insert(node->baseTriangles.end(), tris.begin() + start, tris.begin() + end);
        return node;
    }

    // Choose split axis (longest)
    glm::vec3 extent = bounds.max - bounds.min;
    int axis = 0;
    if (extent.y > extent.x && extent.y > extent.z) axis = 1;
    else if (extent.z > extent.x && extent.z > extent.y) axis = 2;

    // Sort triangles along axis
    std::sort(tris.begin() + start, tris.begin() + end, [axis](const BVHTriangle& a, const BVHTriangle& b) {
        float aCenter = (a.v0[axis] + a.v1[axis] + a.v2[axis]) / 3.0f;
        float bCenter = (b.v0[axis] + b.v1[axis] + b.v2[axis]) / 3.0f;
        return aCenter < bCenter;
    });

    int mid = (start + end) / 2;
    node->left = buildRecursiveTranformed(tris, start, mid, maxLeafSize);
    node->right = buildRecursiveTranformed(tris, mid, end, maxLeafSize);
    return node;
}

BVHNode* BVH::buildRecursive(std::vector<BVHTriangle>& tris, int start, int end, int maxLeafSize) {
    BVHNode* node = new BVHNode();

    AABB bounds;
    for (int i = start; i < end; ++i)
        bounds.expand(computeAABB(tris[i]));
    node->box = bounds;

    int count = end - start;
    if (count <= maxLeafSize) {
        node->isLeaf = true;
        node->baseTriangles.insert(node->baseTriangles.end(), tris.begin() + start, tris.begin() + end);
        return node;
    }

    // Choose split axis (longest)
    glm::vec3 extent = bounds.max - bounds.min;
    int axis = 0;
    if (extent.y > extent.x && extent.y > extent.z) axis = 1;
    else if (extent.z > extent.x && extent.z > extent.y) axis = 2;

    // Sort triangles along axis
    std::sort(tris.begin() + start, tris.begin() + end, [axis](const BVHTriangle& a, const BVHTriangle& b) {
        float aCenter = (a.v0[axis] + a.v1[axis] + a.v2[axis]) / 3.0f;
        float bCenter = (b.v0[axis] + b.v1[axis] + b.v2[axis]) / 3.0f;
        return aCenter < bCenter;
    });

    int mid = (start + end) / 2;
    node->left = buildRecursive(tris, start, mid, maxLeafSize);
    node->right = buildRecursive(tris, mid, end, maxLeafSize);
    return node;
}

void BVH::refitTransformed(const Transform& transform) {
    if (!root) return;

    // Build transformation matrix
    glm::mat4 model = transform.getMatrix();

    // Apply refit recursively
    std::function<AABB(BVHNode*)> refitNode = [&](BVHNode* node) -> AABB {
        if (!node) return AABB();

        node->transformedTriangles.clear();
        
        if (node->isLeaf) {
            AABB leafBox;
            leafBox.min = glm::vec3(std::numeric_limits<float>::max());
            leafBox.max = glm::vec3(std::numeric_limits<float>::lowest());

            for (auto& tri : node->baseTriangles) {
                // Transform each vertex of the triangle
                glm::vec3 v0 = glm::vec3(model * glm::vec4(tri.v0, 1.0f));
                glm::vec3 v1 = glm::vec3(model * glm::vec4(tri.v1, 1.0f));
                glm::vec3 v2 = glm::vec3(model * glm::vec4(tri.v2, 1.0f));

                glm::vec3 worldNormal = normalize(glm::transpose(glm::inverse(glm::mat3(model)))*tri.normal);
            
                AABB triBox;
                triBox.min = glm::min(glm::min(v0, v1), v2);
                triBox.max = glm::max(glm::max(v0, v1), v2);

                leafBox.expand(triBox);
                BVHTriangle transformedTri = {v0, v1, v2, worldNormal, tri.id };
                node->transformedTriangles.push_back(transformedTri);
            }

            node->box = leafBox;
            return leafBox;
        }

        // Refit recursively for internal nodes
        AABB leftBox = refitNode(node->left);
        AABB rightBox = refitNode(node->right);
        node->box = leftBox;
        node->box.expand(rightBox);

        return node->box;
    };

    refitNode(root);
}

void BVH::destroyRecursive(BVHNode* node) {
    if (!node) return;
    destroyRecursive(node->left);
    destroyRecursive(node->right);
    delete node;
}

BVH::~BVH() {
    clear();
}

void BVH::clear() {
    destroyRecursive(root);
    root = nullptr;
}

bool BVH::intersectAABB(const AABB& box) const {
    std::vector<const BVHNode*> stack = { root };
    while (!stack.empty()) {
        const BVHNode* node = stack.back();
        stack.pop_back();

        if (!node->box.intersects(box)) continue;
        if (node->isLeaf) return true;

        if (node->left) stack.push_back(node->left);
        if (node->right) stack.push_back(node->right);
    }
    return false;
}