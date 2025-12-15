#include "mesh_common_shapes.h"
#include "shader.h"
#include "draw.h"
#include "SDL2/SDL.h"
#include "glad/glad.h"
#include <iostream>
#include "init.h"
#include "transform.h"
#include "input.h"
#include "texture.h"
#include "model.h"
#include "bvh.h"
#include "collision.h"
#include "bvh_debug.h"
#include "rigidbody.h"
#include "MeshRenderer.h"
#include "MeshCollider.h"
#include "CubeCollider.h"
#include "cube_collision.h"
#include "InertiaTensor.h"
#include "sweep_and_prune_bp.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

void vec3out(glm::vec3 a){
    std::cout << "(" << a.x << ", " << a.y << ", " << a.z << ")";
}
int main() {
    App app;
    Camera camera;

    if(!init(app)){
        std::cerr << "unable to initialize\n";
    }

    // shaders
    Shader* shader = new Shader("../src/shaders/vertex.glsl","../src/shaders/fragment.glsl");
    Shader* bvhShader = new Shader("../src/shaders/bvh_debug.vert","../src/shaders/bvh_debug.frag");
    initBVHDebug();

    // models loading
    Model cube;
    cube.meshes.push_back(makeCube(1.0f));
    cube.name = "Cube";

    for(auto & mesh : cube.meshes){
        mesh.uploadToGPU();
    }


    // camera setup
    app.relativeMouseMode = true;
    camera.position = glm::vec3(0.0f, 2.0f, 10.0f);
    camera.front = glm::normalize(glm::vec3(0.0f, -0.2f, -1.0f));
    camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.right = glm::normalize(glm::cross(camera.front, camera.up));
    camera.worldUp = camera.up;
    camera.yaw = -90.0f;
    camera.pitch = -10.0f;
    camera.movementSpeed = 5.0f;
    camera.mouseSensitivity = 0.1f;


    // scene setup
    Object* ground = new Object();
    ground->name = "Ground";
    ground->transform->position = glm::vec3(0.0f,-5.0f,0.0f);
    ground->transform->scale = glm::vec3(50.0f,1.0f,50.0f);
    ground->addComponent<MeshRenderer>(&cube, shader);
    ground->addComponent<CubeCollider>(glm::vec3(1.0f,1.0f,1.0f));
    Rigidbody* groundRB = ground->addComponent<Rigidbody>();
    groundRB->isStatic = true;
    groundRB->mass = 100.0f;
    glm::mat3 groundInertia = computeInertiaTensorOBB(glm::vec3(50.0f,1.0f,50.0f), groundRB->mass);
    groundRB->inertiaTensorLocal = groundInertia;
    groundRB->start();
    app.sceneObjects.push_back(ground);


    int num_boxes = 2500;
    for(int i = 0; i<num_boxes; i++){
        Object* boxObj = new Object();
        boxObj->name = "BoxObject_" + std::to_string(i);
        boxObj->transform->position = glm::vec3(
            static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10.0f)) - 5.0f,
            static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/5.0f)) + 5.0f,
            static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10.0f)) - 5.0f
        );
        // boxObj->transform->rotation = glm::quat(glm::vec3(
        //     glm::radians(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/360.0f))),
        //     glm::radians(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/360.0f))),
        //     glm::radians(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/360.0f)))
        // ));
        boxObj->addComponent<MeshRenderer>(&cube, shader);
        CubeCollider* boxCol = boxObj->addComponent<CubeCollider>(glm::vec3(1.0f));
        float mass = 1.0f;
        glm::mat3 inertia = computeInertiaTensorOBB(boxCol->halfExtents, mass);
        Rigidbody* rb = boxObj->addComponent<Rigidbody>();
        rb->mass = mass;
        rb->useGravity = true;
        rb->inertiaTensorLocal = inertia;
        rb->start();
        app.sceneObjects.push_back(boxObj);
    }

    // Object* cubeObj = new Object();
    // cubeObj->name = "CubeObject";
    // cubeObj->transform->rotation = glm::quat(glm::vec3(glm::radians(40.0f),glm::radians(0.0f),glm::radians(0.0f)));
    // // cubeObj->transform->rotation = glm::quat(glm::vec3(glm::radians(45.0f),glm::radians(0.0f),glm::radians(0.0f)));
    // cubeObj->transform->position = glm::vec3(0.0f,10.0f,5.0f);
    // cubeObj->addComponent<MeshRenderer>(&cube, shader);
    // // cubeObj->addComponent<MeshCollider>(&cube, false);
    // CubeCollider* cubeCol1 = cubeObj->addComponent<CubeCollider>(glm::vec3(1.0f));
    // float mass1 = 1.0f;
    // glm::mat3 inertia1 = computeInertiaTensorOBB(cubeCol1->halfExtents, mass1);
    // Rigidbody* rb1 = cubeObj->addComponent<Rigidbody>();
    // // rb1->isStatic = true;
    // rb1->mass = mass1;
    // rb1->useGravity = true;
    // rb1->inertiaTensorLocal = inertia1;
    // rb1->start();
    // app.sceneObjects.push_back(cubeObj);

    // Object* cubeObj2 = new Object();
    // cubeObj2->name = "CubeObject2";
    // cubeObj2->addComponent<MeshRenderer>(&cube, shader);
    // // cubeObj->addComponent<MeshCollider>(&cube, false);
    // CubeCollider* cubeCol2 = cubeObj2->addComponent<CubeCollider>(glm::vec3(1.0f));
    // float mass2 = 1.0f;
    // glm::mat3 inertia2 = computeInertiaTensorOBB(cubeCol2->halfExtents, mass2);
    // Rigidbody* rb2 = cubeObj2->addComponent<Rigidbody>();
    // rb2->mass = mass2;
    // rb2->useGravity = true;
    // rb2->inertiaTensorLocal = inertia2;
    // rb2->start();
    // // cubeObj2->transform->rotation = glm::quat(glm::vec3(glm::radians(45.0f),glm::radians(0.0f),0.0f));
    // cubeObj2->transform->position = glm::vec3(0.0f,0.5f,-5.0f);
    // // cubeObj2->transform->position = glm::vec3(0.3f,0.4f,-50.0f);
    // float cubeScale = 1.0f;
    // cubeObj2->transform->scale = glm::vec3(cubeScale,cubeScale,cubeScale);
    // rb2->velocity = glm::vec3(0.0f,0.0f,5.0f);
    
    // app.sceneObjects.push_back(cubeObj2);


    Object* light1 = new Object();
    light1->name = "Light1";
    auto pointLight = light1->addComponent<PointLight>();
    pointLight->color = glm::vec3(1.0f,1.0f,1.0f);
    pointLight->intensity = 1.0f;
    light1->transform->position = glm::vec3(2.0f,2.0f,2.0f);
    app.sceneObjects.push_back(light1);

    // shader config
    shader->use();
    int toonBands = 64;
    int colorBands = 64;
    float ambientLightIntensity = 1.0f;
    bool useBlinn = true;
    float shininess = 10.0f;
    shader->setInt("toonBands", toonBands);
    shader->setInt("colorBands", colorBands);
    shader->setFloat("ambientLightIntensity", ambientLightIntensity);
    shader->setBool("useBlinn", useBlinn);
    shader->setFloat("shininess", shininess);
    
    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(app.window, app.glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    float time = 0.0f;
    float prevTime = 0.0f;


    // add colliders for sweep and prune
    AABBSorter aabb_sorter(AABBSorter::X);
    for(auto obj: app.sceneObjects){
        if(auto col = obj->getComponent<CubeCollider>()){
            aabb_sorter.addCollider(col);
        }
    }
    aabb_sorter.update();
    // main loop
    while (app.running) {
        // Calculate delta time
        time = (float)SDL_GetTicks() / 1000.0f;
        float deltaTime = time - prevTime;
        prevTime = time;

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Panel");
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        drawEditorUI(app);

        // Handle input
        handleInput(camera, app);

        // collision detection and response


        aabb_sorter.update();
        std::vector<std::pair<Collider*, Collider*>> colliders;
        aabb_sorter.computePairs(colliders);

        for(auto p : colliders){
            resolveCubeCubeCollision(*(CubeCollider*)(p.first),*(CubeCollider*)(p.second),0.5);
        }
        // std::vector<CubeCollider*> cubeColliders;
        // for(auto obj : app.sceneObjects){
        //     if(auto cubeCol = obj->getComponent<CubeCollider>()){
        //         cubeColliders.push_back(cubeCol);
        //     }
        // }
        // for(size_t i=0;i<cubeColliders.size();i++){
        //     for(size_t j=i+1;j<cubeColliders.size();j++){
        //         resolveCubeCubeCollision(*cubeColliders[i], *cubeColliders[j], 0.0f, false);
        //     }
        // }

        // for(size_t j=1;j<cubeColliders.size();j++){
        //     resolveCubeCubeCollision(*cubeColliders[0], *cubeColliders[j], 0.5f, false);
        // }


        // Rendering
        uploadLights(shader, app);
        drawSetup(shader, camera, app);
        for(auto& obj : app.sceneObjects){
            obj->update(deltaTime);
        }

        // Render ImGui UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(app.window);
    }

    cleanupBVHDebug();
    delete shader;
    shader = nullptr;
    for(auto o : app.sceneObjects)delete o;
    app.sceneObjects.clear();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(app.glContext);
    SDL_DestroyWindow(app.window);
    SDL_Quit();

    return 0;
}
