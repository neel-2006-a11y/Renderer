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
#include "DirectionalLight.h"
#include "ShadowMap.h"
#include "helpers.h"
#include "cameraHelpers.h"
#include "app.h"



#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include "SDL2/SDL.h"
#include "glad/glad.h"
#include <iostream>

int main() {
    App& app = App::instance();
    AssetManager& assetManager = AssetManager::instance();
    Renderer renderer;

    if(!init(app)){
        std::cerr << "unable to initialize\n";
    }

    // ===== shaders =====
    Shader* shader = new Shader("../src/shaders/vertex.glsl","../src/shaders/fragment.glsl");
    // Shader* shadowShader = new Shader("../src/shaders/Shadow.vert");
    Shader* shadowShader = new Shader("../src/shaders/Shadow.vert", "../src/shaders/Shadow.frag");
    // ===== models loading =====
    ModelHandle cubeHandle;
    cubeHandle.assetID = "builtin:cube"; 

    // ===== camera setup =====
    Object* camObj = new Object();
    camObj->name = "camObj";
    Camera* camComp = camObj->addComponent<Camera>();
    camObj->addComponent<CameraController>();
    app.sceneObjects.push_back(camObj);


    // ===== scene setup =====
    Object* ground = new Object();
    ground->name = "Ground";
    ground->transform->position = glm::vec3(0.0f,-5.0f,0.0f);
    ground->transform->scale = glm::vec3(50.0f,1.0f,50.0f);
    ground->addComponent<MeshRenderer>(cubeHandle, shader);
    ground->addComponent<CubeCollider>(glm::vec3(1.0f,1.0f,1.0f));
    Rigidbody* groundRB = ground->addComponent<Rigidbody>();
    groundRB->isStatic = true;
    groundRB->mass = 10000.0f;
    glm::mat3 groundInertia = computeInertiaTensorOBB(glm::vec3(50.0f,1.0f,50.0f), groundRB->mass);
    groundRB->inertiaTensorLocal = groundInertia;
    groundRB->start();
    app.sceneObjects.push_back(ground);

    Object* box = new Object();
    box->name = "box";
    box->addComponent<MeshRenderer>(cubeHandle, shader);
    box->transform->rotation = glm::quat(glm::vec3(45,45,90));
    box->transform->scale = glm::vec3(4,0.02,0.02);
    box->transform->position = glm::vec3(15,-3.5,15);
    app.sceneObjects.push_back(box);

    int num_boxes = 100;
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

    // ===== ligths =====
    Object* light1 = new Object();
    light1->name = "Light1";
    auto pointLight = light1->addComponent<PointLight>();
    pointLight->color = glm::vec3(1.0f,1.0f,1.0f);
    pointLight->intensity = 1.0f;
    light1->transform->position = glm::vec3(2.0f,2.0f,2.0f);
    app.sceneObjects.push_back(light1);

    Object* mainLight = new Object();
    mainLight->name = "mainLight";
    DirectionalLight* dirLight = mainLight->addComponent<DirectionalLight>();
    // dirLight->shadowSize = 200.0f;
    ShadowMap mainShadowMap;
    mainShadowMap.resolution = 8192;
    mainShadowMap.init(mainShadowMap.resolution);
    mainLight->transform->rotation = glm::quat(glm::vec3(-45,0,0));
    app.sceneObjects.push_back(mainLight);
    

    // ===== shader config =====
    shader->use();
    int toonBands = 255;
    int colorBands = 255;
    float ambientLightIntensity = 0.2f;
    bool useBlinn = true;
    float shininess = 10.0f;
    shader->setInt("toonBands", toonBands);
    shader->setInt("colorBands", colorBands);
    shader->setFloat("ambientLightIntensity", ambientLightIntensity);
    shader->setBool("useBlinn", useBlinn);
    shader->setFloat("shininess", shininess);
    
    // ===== Setup ImGui context =====
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(app.window, app.glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    float time = 0.0f;
    float prevTime = 0.0f;


    // ===== add colliders for sweep and prune =====
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
        // ImGui::DragFloat("shadowSize", &dirLight->shadowSize, 0.1f);

        // collision detection and response
        aabb_sorter.update();
        std::vector<std::pair<Collider*, Collider*>> colliders;
        aabb_sorter.computePairs(colliders);

        // std::cout << "FLAG2\n";
        for(auto p : colliders){
            resolveCubeCubeCollision(*(CubeCollider*)(p.first),*(CubeCollider*)(p.second),0.5, 4);
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

        // frustrum matching
        // std::vector<glm::vec3> frustumCorners = getCameraFrustumCorners(*activeCamera);
        // dirLight->updateLightMatrices(frustumCorners);

        // handle input 
        handleInput(app);
        handleResize(app);


        // ===== Rendering =====

        // ===== render shadowMap =====
        glm::vec3 sceneCenter = glm::vec3(0);
        mainLight->getComponent<DirectionalLight>()->updateLightMatrices(sceneCenter);

        renderer.renderShadowMap(mainLight->getComponent<DirectionalLight>(), mainShadowMap, app.sceneObjects, shadowShader);
        uploadDirectionalLight(shader, mainLight->getComponent<DirectionalLight>(), &mainShadowMap);
        // uploadLights(shader, app);
        // ===== per-frame clear =====
        glViewport(0,0,app.width, app.height);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
                mr->shader->setVec3("viewPos", activeCamera->owner->transform->position);
                renderer.drawModel(gpuModel);
            }
        }

        for(auto& obj : app.sceneObjects){
            obj->update(deltaTime);
        }

        // Render ImGui UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(app.window);
    }

    delete shader;
    shader = nullptr;
    delete shadowShader;
    shadowShader = nullptr;

    mainShadowMap.destroy();

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
