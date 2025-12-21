#pragma once
#include <vector>
#include <string>
#include "MeshData.h"

struct ModelData {
    std::string assetID;              // path or UUID
    std::vector<MeshData> meshes;
};
