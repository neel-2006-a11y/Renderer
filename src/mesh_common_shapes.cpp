#include "mesh_common_shapes.h"
#include <cmath>

// Cube generator
MeshData makeCube(float size) {
    MeshData mesh;
    float h = size / 2.0f;

    glm::vec3 positions[] = {
        // Front
        {-h, -h,  h}, { h, -h,  h}, { h,  h,  h}, { -h,  h,  h},
        // Back
        {-h, -h, -h}, { h, -h, -h}, { h,  h, -h}, { -h,  h, -h}
    };

    glm::vec3 normals[] = {
        {0, 0, 1}, {0, 0, -1}, {1, 0, 0},
        {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}
    };

    glm::vec2 uvs[] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
    };

    glm::vec3 colors[] = {
        {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0},
        {1, 0, 1}, {0, 1, 1}
    };

    // Each face has 4 vertices and a distinct normal
    // (front, back, right, left, top, bottom)
    struct Face {
        int v[4];
        glm::vec3 n;
        glm::vec3 c;
    };

    Face faces[] = {
        {{0, 1, 2, 3}, normals[0], colors[0]}, // front
        {{5, 4, 7, 6}, normals[1], colors[1]}, // back
        {{1, 5, 6, 2}, normals[2], colors[2]}, // right
        {{4, 0, 3, 7}, normals[3], colors[3]}, // left
        {{3, 2, 6, 7}, normals[4], colors[4]}, // top
        {{4, 5, 1, 0}, normals[5], colors[5]}  // bottom
    };

    for (auto& f : faces) {
        for (int i = 0; i < 4; i++) {
            Vertex v;
            v.position = positions[f.v[i]];
            v.normal = f.n;
            v.color = f.c;
            v.uv = uvs[i];
            mesh.vertices.push_back(v);
        }
    }

    // indices (two triangles per face)
    for (unsigned int i = 0; i < 6; i++) {
        unsigned int start = i * 4;
        mesh.indices.insert(mesh.indices.end(), {
            start, start + 1, start + 2,
            start, start + 2, start + 3
        });
    }

    return mesh;
}

// Sphere generator
MeshData makeSphere(float radius, unsigned int sectors, unsigned int stacks) {
    MeshData mesh;
    const float PI = 3.14159265359f;

    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * (PI / stacks);
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * (2 * PI / sectors);

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            glm::vec3 pos = {x, y, z};
            glm::vec3 norm = glm::normalize(pos);
            glm::vec2 uv = {
                (float)j / sectors,
                (float)i / stacks
            };

            Vertex v;
            v.position = pos;
            v.normal = norm;
            v.color = glm::vec3(1.0f); // white
            v.uv = uv;
            mesh.vertices.push_back(v);
        }
    }

    // indices
    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + sectors + 1;

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0)
                mesh.indices.insert(mesh.indices.end(), {k1, k2, k1 + 1});
            if (i != (stacks - 1))
                mesh.indices.insert(mesh.indices.end(), {k1 + 1, k2, k2 + 1});
        }
    }

    return mesh;
}
