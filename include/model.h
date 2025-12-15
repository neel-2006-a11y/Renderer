#pragma once
#include <string>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "mesh.h"

struct Model{
    std::vector<Mesh> meshes;
    Transform transform;
    std::string name;
};

std::vector<Mesh> loadModel(const std::string& path);
void drawModel(const Model& model, const glm::mat4& parentTransform, Shader* shader);
