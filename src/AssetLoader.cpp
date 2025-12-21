#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "MeshData.h"
#include "AssetLoader.h"
#include "mesh_common_shapes.h"
#include <iostream>

AssetLoader& AssetLoader::instance(){
    static AssetLoader al;
    return al;
}
ModelData* AssetLoader::loadModelFromFile(const std::string& path) {
    ModelData result;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return &result;
    }

    std::function<void(aiNode*, const aiScene*)> processNode;
    processNode = [&](aiNode* node, const aiScene* scene) {
        // Process all the node's meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            MeshData myMesh;

            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                Vertex vertex;
                vertex.position = glm::vec3(
                    mesh->mVertices[v].x,
                    mesh->mVertices[v].y,
                    mesh->mVertices[v].z);

                vertex.normal = mesh->HasNormals() ?
                    glm::vec3(mesh->mNormals[v].x,
                              mesh->mNormals[v].y,
                              mesh->mNormals[v].z) :
                    glm::vec3(0.0f);

                if (mesh->mTextureCoords[0]) {
                    vertex.uv = glm::vec2(
                        mesh->mTextureCoords[0][v].x,
                        mesh->mTextureCoords[0][v].y);
                } else {
                    vertex.uv = glm::vec2(0.0f);
                }

                myMesh.vertices.push_back(vertex);
            }

            for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                aiFace face = mesh->mFaces[f];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    myMesh.indices.push_back(face.mIndices[j]);
                }
            }

            result.meshes.push_back(myMesh);
        }

        // Process all children recursively
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    };

    processNode(scene->mRootNode, scene);

    return &result;
}

ModelData* AssetLoader::loadBuiltin(const std::string& assetID){
    if(assetID == "builtin:cube"){
        ModelData* m = new ModelData();
        m->meshes.push_back(makeCube());
        return m;
    }
    assert(false && "Unknown builtin meshs");
}