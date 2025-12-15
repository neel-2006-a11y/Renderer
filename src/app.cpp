#include "app.h"
#include "mesh.h"
#include "pointLight.h"
#include "Component.h"
#include <imgui.h>

void drawEditorUI(App& app) {
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
        ImGui::DragFloat3("Rotation", &object->transform->rotation[0], 0.1f);
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
