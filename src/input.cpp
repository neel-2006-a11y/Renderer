#include "input.h"
#include "camera.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>

void handleInput(App& app) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_RESIZED) {

            app.width  = event.window.data1;
            app.height = event.window.data2;

            glViewport(0,0,app.width,app.height);

            for (Object* obj : app.sceneObjects) {
                if (auto cam = obj->getComponent<Camera>()) {
                    cam->projDirty = true;
                }
            }
        }
    }
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_ESCAPE]) {
        app.running = false;
    }

}
