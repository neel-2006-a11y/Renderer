#include "bvh_debug.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

// -------------------- Line vertex struct --------------------
struct LineVertex {
    glm::vec3 position;
    glm::vec3 color;
};

// -------------------- Static GL handles --------------------
static unsigned int debugVAO = 0, debugVBO = 0;
static bool initialized = false;

// -------------------- Initialize buffers --------------------
void initBVHDebug() {
    if (initialized) return;
    glGenVertexArrays(1, &debugVAO);
    glGenBuffers(1, &debugVBO);
    initialized = true;
}

// -------------------- AABB to line vertices --------------------
std::vector<LineVertex> makeAABBWireframe(const AABB& box, const glm::vec3& color) {
    glm::vec3 min = box.min;
    glm::vec3 max = box.max;

    glm::vec3 v[8] = {
        {min.x, min.y, min.z}, {max.x, min.y, min.z},
        {max.x, max.y, min.z}, {min.x, max.y, min.z},
        {min.x, min.y, max.z}, {max.x, min.y, max.z},
        {max.x, max.y, max.z}, {min.x, max.y, max.z}
    };

    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    std::vector<LineVertex> verts;
    verts.reserve(24);
    for (auto& e : edges) {
        verts.push_back({v[e[0]], color});
        verts.push_back({v[e[1]], color});
    }

    return verts;
}

// -------------------- Collect AABBs up to given depth --------------------
void collectBVHBoxes(BVHNode* node, std::vector<AABB>& out, int depth, int maxDepth) {
    if (!node || depth > maxDepth) return;
    out.push_back(node->box);
    collectBVHBoxes(node->left, out, depth + 1, maxDepth);
    collectBVHBoxes(node->right, out, depth + 1, maxDepth);
}

// -------------------- Draw all AABB lines --------------------
void drawBVHDebug(Camera& camera, App& app, BVH& bvh, Shader* shader, int maxDepth) {
    if (!initialized || !bvh.root) return;

    std::vector<AABB> boxes;
    collectBVHBoxes(bvh.root, boxes, 0, maxDepth);

    std::vector<LineVertex> allVerts;
    for (size_t i = 0; i < boxes.size(); ++i) {
        float t = (float)i / (float)boxes.size();
        glm::vec3 color = glm::mix(glm::vec3(1, 1, 0), glm::vec3(1, 0, 0), t);
        auto verts = makeAABBWireframe(boxes[i], color);
        allVerts.insert(allVerts.end(), verts.begin(), verts.end());
    }

    glBindVertexArray(debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
    glBufferData(GL_ARRAY_BUFFER, allVerts.size() * sizeof(LineVertex), allVerts.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)(offsetof(LineVertex, color)));

    shader->use();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)app.width / app.height, 0.1f, 100.0f);
    glm::mat4 viewProj = projection * view;
    shader->setMat4("vp", &viewProj[0][0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_LINES, 0, (GLsizei)allVerts.size());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
}

// -------------------- Cleanup --------------------
void cleanupBVHDebug() {
    if (!initialized) return;
    glDeleteVertexArrays(1, &debugVAO);
    glDeleteBuffers(1, &debugVBO);
    initialized = false;
}
