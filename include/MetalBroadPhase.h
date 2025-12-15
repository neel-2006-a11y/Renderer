#pragma once
#include <vector>
#include "CC_broadphase.h"

void initMetalBroadPhase(int count);

std::vector<CollisionPair> runBroadPhaseGPU(
    const std::vector<AABB_GPU>& aabbs
);