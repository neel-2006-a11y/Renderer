#pragma once
#include "glm/glm.hpp"
#include "camera.h"

std::vector<glm::vec3> getCameraFrustumCorners(const Camera& cam);