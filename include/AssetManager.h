#pragma once
#include <unordered_map>
#include <string>
#include "ModelData.h"

class AssetManager {
public:
    static AssetManager& instance();

    ModelData* getModel(const std::string& assetID);

private:
    std::unordered_map<std::string, ModelData*> modelCache;
};
