#pragma once
#include "glm/glm.hpp"
#include "camera.h"

std::vector<glm::vec3> getCameraFrustumCorners(const Camera& cam);
void getCameraFrustumCorners(const Camera& cam, float nearPlane, float farPlane, std::vector<glm::vec3>& outCorners);