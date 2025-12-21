#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "MeshData.h"

// Generates a cube centered at origin with given size
MeshData makeCube(float size = 1.0f);

// Generates a UV sphere centered at origin
// sectors = longitude divisions, stacks = latitude divisions
MeshData makeSphere(float radius = 1.0f, unsigned int sectors = 32, unsigned int stacks = 16);

