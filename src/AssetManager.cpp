#include "AssetManager.h"
#include "AssetLoader.h"
#include "mesh_common_shapes.h"
#include <cassert>
#include <string>

AssetManager& AssetManager::instance() {
    static AssetManager inst;
    return inst;
}

bool isBuiltin(const std::string& s) {
    const std::string prefix = "builtin:";
    return s.size() >= prefix.size() && s.compare(0,prefix.size(), prefix) == 0;
}

ModelData* AssetManager::getModel(const std::string& assetID) {
    if (modelCache.count(assetID))
        return modelCache[assetID];

    ModelData* model = nullptr;
    AssetLoader& al = AssetLoader::instance();
    if (isBuiltin(assetID))
        model = al.loadBuiltin(assetID);
    else
        model = al.loadBuiltin(assetID);

    model->assetID = assetID;
    modelCache[assetID] = model;
    return model;
}
