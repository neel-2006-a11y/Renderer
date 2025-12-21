#include "mesh_common_shapes.h"
#include "shader.h"
#include "init.h"
#include "transform.h"
#include "input.h"
#include "texture.h"
#include "rigidbody.h"
#include "MeshRenderer.h"
#include "CubeCollider.h"
#include "pointLight.h"
#include "OBBCollision.h"
#include "InertiaTensor.h"
#include "sweep_and_prune_bp.h"
#include "ModelData.h"
#include "GPUModel.h"
#include "ModelHandle.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "CameraController.h"


#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include "SDL2/SDL.h"
#include "glad/glad.h"
#include <iostream>

void vec3out(glm::vec3 a){
    std::cout << "(" << a.x << ", " << a.y << ", " << a.z << ")";
}
int main() {
    App& app = App::instance();
    AssetManager& assetManager = AssetManager::instance();
    Renderer renderer;

    if(!init(app)){
        std::cerr << "unable to initialize\n";
    }

    // shaders
    Shader* shader = new Shader("../src/shaders/vertex.glsl","../src/shaders/fragment.glsl");

    // models loading
    ModelHandle cubeHandle;
    cubeHandle.assetID = "builtin:cube"; 

    // camera setup
    Object* camObj = new Object();
    camObj->name = "camObj";
    camObj->addComponent<Camera>();
    camObj->addComponent<CameraController>();
    app.sceneObjects.push_back(camObj);

    
    // scene setup
    Object* ground = new Object();
    ground->name = "Ground";
    ground->transform->position = glm::vec3(0.0f,-5.0f,0.0f);
    ground->transform->scale = glm::vec3(50.0f,1.0f,50.0f);
    ground->addComponent<MeshRenderer>(cubeHandle, shader);
    ground->addComponent<CubeCollider>(glm::vec3(1.0f,1.0f,1.0f));
    Rigidbody* groundRB = ground->addComponent<Rigidbody>();
    groundRB->isStatic = true;
    groundRB->mass = 100.0f;
    glm::mat3 groundInertia = computeInertiaTensorOBB(glm::vec3(50.0f,1.0f,50.0f), groundRB->mass);
    groundRB->inertiaTensorLocal = groundInertia;
    groundRB->start();
    app.sceneObjects.push_back(ground);


    int num_boxes = 1000;
    for(int i = 0; i<num_boxes; i++){
        Object* boxObj = new Object();
        boxObj->name = "BoxObject_" + std::to_string(i);
        boxObj->transform->position = glm::vec3(
            static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/50.0f))-25,
            static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/5.0f)) + 10.0f,
            static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/50.0f))-25
        );
        // boxObj->transform->rotation = glm::quat(glm::vec3(
        //     glm::radians(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/360.0f))),
        //     glm::radians(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/360.0f))),
        //     glm::radians(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/360.0f)))
        // ));
        boxObj->addComponent<MeshRenderer>(cubeHandle, shader);
        CubeCollider* boxCol = boxObj->addComponent<CubeCollider>(glm::vec3(1.0f));
        float mass = 1.0f;
        glm::mat3 inertia = computeInertiaTensorOBB(boxCol->halfExtents, mass);
        Rigidbody* rb = boxObj->addComponent<Rigidbody>();
        rb->velocity = glm::vec3(2.0,0,0);
        rb->mass = mass;
        rb->useGravity = true;
        rb->inertiaTensorLocal = inertia;
        rb->start();
        app.sceneObjects.push_back(boxObj);
    }

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
    float ambientLightIntensity = 10.0f;
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

    // std::cout << "FLAG1\n";


    // ===== main loop =====
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

        // collision detection and response
        aabb_sorter.update();
        std::vector<std::pair<Collider*, Collider*>> colliders;
        aabb_sorter.computePairs(colliders);

        // std::cout << "FLAG2\n";
        for(auto p : colliders){
            resolveCubeCubeCollision(*(CubeCollider*)(p.first),*(CubeCollider*)(p.second),0.5, 1);
        }

        // get active camera
        Camera* activeCamera = nullptr;
        for(auto& obj : app.sceneObjects){
            activeCamera = obj->getComponent<Camera>();
            if(activeCamera != nullptr){
                break;
            }
        }
        if(!activeCamera){
            std::cerr << "no active camera\n";
            return 0;
        }

        // handle input 
        handleInput(app);
        // std::cout << app.running << "\n";
        // Rendering

        // ===== per-frame clear =====
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        uploadLights(shader, app);
        

        // std::cout << "FLAG3\n";
        for(auto& obj : app.sceneObjects){
            if(auto mr = obj->getComponent<MeshRenderer>()){
                mr->shader->use();
                ModelData* modelData = assetManager.getModel(mr->model.assetID);
                GPUModel* gpuModel = renderer.getGPUModel(modelData);
                glm::mat4 view = activeCamera->getView();
                glm::mat4 proj = activeCamera->getProjection();
                glm::mat4 model = mr->owner->transform->getMatrix();
                mr->shader->setMat4("view", &view[0][0]);
                mr->shader->setMat4("projection", &proj[0][0]);
                mr->shader->setMat4("model", &model[0][0]);
                renderer.drawModel(gpuModel);
            }
        }

        // std::cout << "FLAG4\n";
        for(auto& obj : app.sceneObjects){
            obj->update(deltaTime);
        }
        // std::cout << "FLAG5\n";
        // Render ImGui UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // std::cout << "FLAG6\n";
        SDL_GL_SwapWindow(app.window);
        // std::cout << "FLAG7\n";
    }

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
