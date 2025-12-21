#pragma once
#include <unordered_map>
#include "ModelData.h"
#include "GPUModel.h"
#include "camera.h"

class Renderer {
public:
    GPUModel* getGPUModel(ModelData* model);
    void drawModel(GPUModel* model);

    void beginFrame(const Camera& camera);

private:
    GPUMesh uploadMesh(const MeshData& mesh);

    std::unordered_map<ModelData*, GPUModel*> gpuCache;
};
