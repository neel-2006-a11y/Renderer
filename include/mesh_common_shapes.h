#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "mesh.h"

// Generates a cube centered at origin with given size
Mesh makeCube(float size = 1.0f);

// Generates a UV sphere centered at origin
// sectors = longitude divisions, stacks = latitude divisions
Mesh makeSphere(float radius = 1.0f, unsigned int sectors = 32, unsigned int stacks = 16);

