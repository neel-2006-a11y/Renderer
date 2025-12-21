#pragma once
#include "bvh.h"
#include "app.h"
#include "camera.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <vector>

void initBVHDebug();
void drawBVHDebug(Camera& camera, App& app, BVH& bvh, Shader* shader, int maxDepth);
void cleanupBVHDebug();
