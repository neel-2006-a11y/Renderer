#pragma once
#include <vector>
#include "app.h"
#include "mesh.h"
#include "transform.h"
#include "pointLight.h"
#include "camera.h"

void drawSetup(Shader* shader, Camera& camera, App& app);
void drawMesh(Shader* shader, const Mesh& mesh, const glm::mat4& transform);