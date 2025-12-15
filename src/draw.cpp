#include "draw.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void drawSetup(Shader* shader, Camera& camera, App& app) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->use();

    // Basic view/projection setup
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)app.width / app.height, 0.1f, 100.0f);

    shader->setVec3("viewPos", camera.position);
    shader->setMat4("view", &view[0][0]);
    shader->setMat4("projection", &projection[0][0]);
}

void drawMesh(Shader* shader, const Mesh& mesh, const glm::mat4& transform)
{
    if(mesh.uploaded == false){
        std::cerr << "Mesh not uploaded to GPU!\n";
        return;
    }
    shader->use();
    shader->setMat4("model", &transform[0][0]);

    if (mesh.colorTexture.id != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.colorTexture.id);
        shader->setInt("diffuseTex", 0);
        shader->setBool("useTexture", true);
    } else {
        shader->setBool("useTexture", false);
    }

    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES,
                   (GLsizei)mesh.indices.size(),
                   GL_UNSIGNED_INT,
                   0);
    glBindVertexArray(0);
}
