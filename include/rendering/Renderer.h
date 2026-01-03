#pragma once
#include <unordered_map>
#include "ModelData.h"
#include "GPUModel.h"
#include "camera.h"
#include "DirectionalLight.h"
#include "ShadowMap.h"
#include "Object.h"
#include "shader.h"

class Renderer {
public:
    GPUModel* getGPUModel(ModelData* model);
    void drawModel(GPUModel* model);

    void beginFrame(const Camera& camera);
    void renderShadowMap(DirectionalLight* light, ShadowMap& shadowMap, const std::vector<Object*>& objects, Shader* shadowShader);

private:
    GPUMesh uploadMesh(const MeshData& mesh);

    std::unordered_map<ModelData*, GPUModel*> gpuCache;
};
