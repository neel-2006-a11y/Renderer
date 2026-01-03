#pragma once

#include "glad/glad.h"

struct ShadowMap{
    GLuint fbo = 0;
    GLuint depthTexture = 0;
    int resolution = 2048;

    void init(int res);
    void destroy();
};