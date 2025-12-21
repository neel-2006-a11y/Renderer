#pragma once
#include <glad/glad.h>

struct GPUMesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    uint32_t indexCount = 0;
};
