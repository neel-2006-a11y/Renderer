#pragma once
#include <SDL2/SDL.h>
#include "Object.h"
#include "shader.h"

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