#include "input.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL2/SDL.h>

void handleInput(Camera& camera, App& app) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Guid input handling
        ImGui_ImplSDL2_ProcessEvent(&event);
        if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB){
            app.relativeMouseMode = !app.relativeMouseMode;
            SDL_SetRelativeMouseMode(app.relativeMouseMode ? SDL_TRUE : SDL_FALSE);
        }


        if (event.type == SDL_QUIT) {
            app.running = false;
        }
        // move camera with mouse
        else if (event.type == SDL_MOUSEMOTION) {
            if(app.relativeMouseMode){
                float xpos = static_cast<float>(event.motion.x);
                float ypos = static_cast<float>(event.motion.y);

                float xoffset = (float)event.motion.xrel;
                float yoffset = (float)event.motion.yrel;

                camera.processMouseMovement(xoffset, yoffset);
            }
        }
    }

    // Keyboard movement
    const Uint8* state = SDL_GetKeyboardState(NULL);
    camera.updateDeltaTime();

    if (state[SDL_SCANCODE_W])
        camera.processKeyboard(FORWARD);
    if (state[SDL_SCANCODE_S])
        camera.processKeyboard(BACKWARD);
    if (state[SDL_SCANCODE_A])
        camera.processKeyboard(LEFT);
    if (state[SDL_SCANCODE_D])
        camera.processKeyboard(RIGHT);

    // Escape key closes window
    if (state[SDL_SCANCODE_ESCAPE])
        app.running = false;
}
