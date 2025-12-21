#include "Renderer.h"

GPUMesh Renderer::uploadMesh(const MeshData& mesh) {
    GPUMesh g;

    glGenVertexArrays(1, &g.vao);
    glGenBuffers(1, &g.vbo);
    glGenBuffers(1, &g.ebo);

    glBindVertexArray(g.vao);

    glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
    glBufferData(GL_ARRAY_BUFFER,
        mesh.vertices.size() * sizeof(Vertex),
        mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh.indices.size() * sizeof(uint32_t),
        mesh.indices.data(),
        GL_STATIC_DRAW
    );

    // vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, uv));

    g.indexCount = mesh.indices.size();
    return g;
}

GPUModel* Renderer::getGPUModel(ModelData* model) {
    if (gpuCache.count(model))
        return gpuCache[model];

    GPUModel* gpu = new GPUModel();
    for (auto& m : model->meshes)
        gpu->meshes.push_back(uploadMesh(m));

    gpuCache[model] = gpu;
    return gpu;
}

void Renderer::drawModel(GPUModel* model) {
    for (auto& mesh : model->meshes) {
        glBindVertexArray(mesh.vao);
        glDrawElements(
            GL_TRIANGLES,
            mesh.indexCount,
            GL_UNSIGNED_INT,
            nullptr
        );
    }
}

void Renderer::beginFrame(const Camera& camera){
    glm::mat4 VP = camera.getProjection() * camera.getView();
}