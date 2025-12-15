#include "model.h"
#include "draw.h"
#include "app.h"
#include <iostream>
#include <glm/glm.hpp>

static void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes);
static Mesh processMesh(aiMesh* mesh, const aiScene* scene);

std::vector<Mesh> loadModel(const std::string& path) {
    std::vector<Mesh> meshes;
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return meshes;
    }

    processNode(scene->mRootNode, scene, meshes);
    return meshes;
}

static void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes) {
    // Process all meshes in this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // Recursively process children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes);
    }
}

static Mesh processMesh(aiMesh* mesh, const aiScene* /*scene*/) {
    Mesh resultMesh;
    resultMesh.vertices.reserve(mesh->mNumVertices);
    resultMesh.indices.reserve(mesh->mNumFaces * 3);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};

        vertex.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };

        if (mesh->HasNormals()) {
            vertex.normal = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            };
        }

        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        } else {
            vertex.texCoord = {0.0f, 0.0f};
        }

        vertex.color = {1.0f, 1.0f, 1.0f}; // default white
        resultMesh.vertices.push_back(vertex);
    }

    // Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            resultMesh.indices.push_back(face.mIndices[j]);
    }

    return resultMesh;
}

void drawModel(const Model& model, const glm::mat4& parentTransform, Shader* shader) {
    glm::mat4 combinedTransform = parentTransform * model.transform.getMatrix();
    for (const Mesh& mesh : model.meshes) {
        drawMesh(shader, mesh, parentTransform);
    }
}
