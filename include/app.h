#pragma once
#include <SDL2/SDL.h>
#include "Object.h"
#include "shader.h"
#include "DirectionalLight.h"
#include "DirectionalLightCSM.h"
#include "ShadowMap.h"

class App {
public:
    static App& instance();
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    bool running = true;
    bool relativeMouseMode = false;

    int width = 800;
    int height = 600;
    std::vector<Object*> sceneObjects;
};

void uploadLights(Shader* shader, App& app);
void drawEditorUI(App& app);
void handleResize(App& app);
void uploadDirectionalLight(Shader* shader, DirectionalLight* light, ShadowMap* shadowMap);
void uploadDirectionalLightCSM(Shader* shader, DirectionalLightCSM* light, ShadowMap shadowMaps[]);