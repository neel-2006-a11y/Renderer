#pragma once
#include "ModelData.h"

class AssetLoader{
    public:
    static AssetLoader& instance();
    ModelData* loadModelFromFile(const std::string& path);
    ModelData* loadBuiltin(const std::string& assetID);
}; 