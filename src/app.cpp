#include "app.h"
#include "pointLight.h"
#include "Component.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>

App& App::instance(){
    static App inst;
    return inst;
}

void drawEditorUI(App& app) {

    ImGui::Begin("Debug Panel");
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        
    ImGui::Begin("Scene Editor");

    static int selectedModel = 0;

    // Mesh selection
    for (int i = 0; i < app.sceneObjects.size(); ++i) {
        char label[32];
        if(app.sceneObjects[i]->name.empty())
            sprintf(label, "Model %d", i);
        else
            sprintf(label, "%s", app.sceneObjects[i]->name.c_str());
        if (ImGui::Selectable(label, selectedModel == i))
            selectedModel = i;
    }

    // Transform controls
    if (!app.sceneObjects.empty()) {
        Object* object = app.sceneObjects[selectedModel];

        ImGui::Separator();
        ImGui::Text("Transform");
        ImGui::DragFloat3("Position", &object->transform->position[0], 0.1f);

        // glm::vec3 euler = glm::degrees(glm::eulerAngles(object->transform->rotation));
        // ImGui::DragFloat3("Rotation", &euler[0], 0.1f);
        // object->transform->rotation = glm::quat(glm::radians(euler));
        ImGui::DragFloat3("Scale", &object->transform->scale[0], 0.1f);
    }

    ImGui::End();
}

void uploadLights(Shader* shader, App& app) {
    shader->use();

    std::vector<PointLight> lights;
    // Collect point lights from the scene
    for(auto& obj : app.sceneObjects) {
        if (auto light = obj->getComponent<PointLight>()) {
            lights.push_back(*light);
        }
    }
    // Set number of active point lights
    shader->setInt("numPointLights", static_cast<int>(lights.size()));

    for (size_t i = 0; i < lights.size(); ++i) {
        const PointLight& light = lights[i];

        // Build uniform prefix (e.g., "pointLights[0].position")
        std::string base = "pointLights[" + std::to_string(i) + "]";

        shader->setVec3(base + ".position", light.owner->transform->position);
        shader->setVec3(base + ".color", light.color);
        shader->setFloat(base + ".intensity", light.intensity);
        shader->setFloat(base + ".constant", light.constant);
        shader->setFloat(base + ".linear", light.linear);
        shader->setFloat(base + ".quadratic", light.quadratic);
    }
}

void handleResize(App& app) {
    SDL_GetWindowSize(app.window, &app.width, & app.height);
    glViewport(0,0,app.width,app.height);
    for(auto obj:app.sceneObjects){
        if(auto cam = obj->getComponent<Camera>()){
            cam->projDirty = true;
        }
    }
}

void uploadDirectionalLight(Shader* shader, DirectionalLight* light, ShadowMap* shadowMap){
    shader->use();
    shader->setVec3("dirLight.direction", light->getDirection());
    shader->setVec3("dirLight.color", light->color);
    shader->setFloat("dirLight.intensity", light->intensity);
    shader->setBool("useDirectionalLight", true);
    
    shader->setInt("shadowMap",1);

    glm::mat4 lightVP = light->lightVP;
    // for(int i = 0; i<4; i++){
    //     for(int j = 0; j<4; j++){
    //         std::cout << lightVP[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << std::endl;
    shader->setMat4("lightVP", &lightVP[0][0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMap->depthTexture);
}

void uploadDirectionalLightCSM(Shader* shader, DirectionalLightCSM* lightCSM, ShadowMap shadowMaps[]){
    shader->use();
    shader->setVec3("dirLight.direction", lightCSM->getDirection());
    shader->setVec3("dirLight.color", lightCSM->color);
    shader->setFloat("dirLight.intensity", lightCSM->intensity);
    shader->setInt("NUM_CASCADES", DirectionalLightCSM::NUM_CASCADES);
    shader->setBool("useDirectionalLight", true);

    glm::mat4 lightVPs[DirectionalLightCSM::NUM_CASCADES];
    float cascadeSplits[DirectionalLightCSM::NUM_CASCADES];

    // std::cout << "Cascade splits: ";
    for(int i = 0; i < DirectionalLightCSM::NUM_CASCADES; i++){
        lightVPs[i] = lightCSM->cascades[i].lightVP;
        cascadeSplits[i] = lightCSM->cascades[i].splitFar;

    }

    for(int i = 0; i < DirectionalLightCSM::NUM_CASCADES; i++){
        shader->setMat4("lightVPs[" + std::to_string(i) + "]", &lightVPs[i][0][0]);
        shader->setFloat("cascadeSplits[" + std::to_string(i) + "]", cascadeSplits[i]);
    }
    // glActiveTexture(GL_TEXTURE1);
    for(int i = 0; i < DirectionalLightCSM::NUM_CASCADES; i++){
        glActiveTexture(GL_TEXTURE1 + i);
        glBindTexture(GL_TEXTURE_2D, shadowMaps[i].depthTexture);
        shader->setInt("shadowMaps[" + std::to_string(i) + "]", 1+i);
    }
}