#include "Application.h"

#include "Scene/Scene.h"
#include "Scene/SceneUtils/B1BattleDroid.h"
#include "Scene/SceneUtils/BuzzDroid.h"
#include "Scene/SceneUtils/JediStarfighter.h"
#include "Scene/SceneUtils/TantiveIV.h"
#include "Scene/SceneUtils/Thranta.h"
#include "Scene/SceneUtils/PongKrell.h"
#include "Scene/SceneUtils/Plane.h"

#include "Renderer/Renderer.h"
#include "Renderer/PathTracer.h"
#include "Renderer/WebGPU/wgpuDevice.h"

#include "Renderer/MaterialSystem.h"
#include "Renderer/TextureSystem.h"
#include "Renderer/Geometry/GeometrySystem.h"
#include "Renderer/Pipelines/Procedural/NoisePipeline.h"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <cassert>
#include <cstdio>
#include <memory.h>

#include <glm/gtc/matrix_transform.hpp>

#include <array>

#ifndef __EMSCRIPTEN__
#define EM_ASM(x, y)
#endif

// #define PATH_TRACE

static double elapsed = 0.f;
static double lastTime = 0.f;
static std::array<double, 100> deltaTimes = {};
static uint64_t frameNo = 0;
static int currentFPS = 0.f;

////////////////////////////////////////////////////////////////////////////////////////
// Temporary, will be moved/handled differently
static uint32_t WINDOW_WIDTH = 800;
static uint32_t WINDOW_HEIGHT = 600;
static float aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
static float fovY = glm::radians(45.f);

////////////////////////////////////////////////////////////////////////////////////////

static Application *s_Instance = nullptr;

static WGpuTexture *depthTexture = nullptr;

void materialUpdateDoneCallback(WGPUQueueWorkDoneStatus status, void * userdata)
{
    if(status != WGPUQueueWorkDoneStatus::WGPUQueueWorkDoneStatus_Success){
        printf("Error while waiting for material queue to finish updates.\n");
    }
    s_Instance->renderFrame();
}

void GetDevice()
{
    const WGPUInstance instance = nullptr;

    WGPURequestAdapterOptions *options = (WGPURequestAdapterOptions *)malloc(sizeof(WGPURequestAdapterOptions));
    memset(options, 0, sizeof(WGPURequestAdapterOptions));
    options->powerPreference = WGPUPowerPreference::WGPUPowerPreference_HighPerformance;
    options->forceFallbackAdapter = false;

    wgpuInstanceRequestAdapter(
        instance, options, [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata)
        {

        if(status != WGPURequestAdapterStatus::WGPURequestAdapterStatus_Success){
            printf("Failed to get adapter: %s\n", message);
        }
        else {
            WGPUAdapterProperties* props = (WGPUAdapterProperties*)malloc(sizeof(WGPUAdapterProperties));
            memset(props, 0, sizeof(WGPUAdapterProperties));
            wgpuAdapterGetProperties(adapter, props);

            printf("Adapter properties:\n");
            printf("\tName: %s\n", (props->name ? props->name : ""));
            printf("\tDriver description: %s\n", (props->driverDescription ? props->driverDescription : ""));
            printf("\tVendor id: %u\n", props->vendorID);
            printf("\tDevice id: %u\n", props->deviceID);
            printf("\tAdapter Type: %i\n", static_cast<int>(props->adapterType));
            printf("\tBackend Type: %i\n", static_cast<int>(props-> backendType));
            
            WGPUDeviceDescriptor* deviceDescription = (WGPUDeviceDescriptor*)malloc(sizeof(WGPUDeviceDescriptor));
            memset(deviceDescription, 0, sizeof(WGPUDeviceDescriptor));
            deviceDescription->label = "Graphics device";
            
            wgpuAdapterRequestDevice(adapter, deviceDescription, [](WGPURequestDeviceStatus status, WGPUDevice dev, char const * message, void * userdata){
                if(status != WGPURequestDeviceStatus_Success){
                    printf("Failed to get device from adapter: %s\n", message);
                }
                else {
                    wgpu::Device device_ = wgpu::Device::Acquire(dev);

                    
                    s_Instance->setDevice(device_);
                    s_Instance->initializeAndRun();
                    // reinterpret_cast<void (*)(wgpu::Device)>(userdata)(device_);
                }
            }, userdata);
            
        } },
        nullptr);
}

int newAnimFrame(double t, void* userData) {
    Application::get()->onUpdate();
    return 1;
}

Application::Application(const std::string &applicationName)
{
    assert(!s_Instance);

    lastTime = emscripten_get_now();

    m_Name = applicationName;
    s_Instance = this;

    m_IsInitialized = false;
    m_ActiveScene = nullptr;
    m_Scene_PathTrac = nullptr;
    m_Scene_Raster = nullptr;
    m_State = State::Other;
    // Get the device and then continue with initialization
    GetDevice();
}

Application::~Application()
{
}

void Application::setDevice(wgpu::Device device_)
{
    m_Device = new WGpuDevice(device_);
}

void Application::initializeAndRun()
{
    printf("Do some initialization\n");

    m_MaterialSystem = new MaterialSystem(m_Device);
    m_GeometrySystem = new GeometrySystem(m_Device);
    m_TextureSystem = new TextureSystem(m_Device);

// #ifdef PATH_TRACE
//     startPathTracer();
// #else
    startRasterizer();
    startPathTracer();
// #endif

    onUpdate();
}

void Application::transition(State state)
{
    if(m_State == state) return;

    if(state == State::Other) return;

    m_TransitionOnNextFrame = true;
    m_TargetState = state;
}

void Application::onUpdate()
{
    if (!m_IsInitialized)
        return;

    if(m_TransitionOnNextFrame){
        if(m_TargetState == State::PathTracer){
            //TODO: cleanup the rasterizer state and scene
            // m_IsInitialized = false;
            // delete m_Renderer;
            // m_Renderer = nullptr;
            // delete m_Scene;
            // m_Scene = nullptr;
            

            // m_MaterialSystem->cleanup();
            // m_GeometrySystem->clear();
            // m_TextureSystem->clear();

            // startPathTracer();
            m_ActiveScene = m_Scene_PathTrac;
            m_State = State::PathTracer;
            m_TransitionOnNextFrame = false;
        }

        if(m_TargetState == State::Rasterizer) {
            //TODO: cleanup the path tracer state and scene
            // m_IsInitialized = false;
            // delete m_PathTracer;
            // m_PathTracer = nullptr;
            // delete m_Scene;
            // m_Scene = nullptr;

            // m_MaterialSystem->cleanup();
            // m_GeometrySystem->clear();
            // m_TextureSystem->clear();

            // startRasterizer();
            m_ActiveScene = m_Scene_Raster;
            m_State = State::Rasterizer;
            m_TransitionOnNextFrame = false;
        }
    }

    if (m_ActiveScene)
        m_ActiveScene->onUpdate();


    double now = emscripten_get_now();
    double deltaTime = now-lastTime;

    lastTime = now;
    uint64_t index = frameNo % deltaTimes.size();
    deltaTimes[index] = deltaTime;

    double avgDelta = 0.f;
    for(int i = 0; i < std::fmin(deltaTimes.size(), frameNo+1); ++i)
    {
        avgDelta += deltaTimes[i];
    }

    avgDelta /= double(std::fmin(deltaTimes.size(), frameNo+1));
    avgDelta /= 1000.0;

    currentFPS = int(1.0/avgDelta);
    
    EM_ASM({
        let elem = document.getElementById("FPSdiv");
        elem.innerHTML = "FPS: " + $0;
        }, currentFPS
    );

    if(m_State == State::PathTracer)
    {
        if(m_PathTracer){
            renderFrame();
        }
    }
    else{
        if (m_Renderer)
        {
            wgpu::Queue materialQueue = m_Device->getHandle().GetQueue();

            bool waitForUpdate = m_MaterialSystem->onUpdate(m_Device, &materialQueue);
            if(waitForUpdate){
                //TODO: instead of waiting here we should do it just in time before we do the lighting pass
                materialQueue.OnSubmittedWorkDone(0, materialUpdateDoneCallback, nullptr);
            }
            else {
                // No updates pending to materials. We can start rendering immediately
                renderFrame();
            }        
        }
    }

    ++frameNo;
}

void Application::renderFrame()
{
    if(m_State == State::PathTracer)
    {
        m_PathTracer->run();
    }
    else if(m_State == State::Rasterizer)
    {
        m_Renderer->render(m_ActiveScene);
    }
}

void Application::startPathTracer()
{
    SceneDescription scene{};
    scene.name = "Path Tracer Test Scene";
    std::vector<ModelDescription> models;
    std::vector<MaterialDescription> materials;

    // m_State = State::PathTracer;

    scene.modelDescriptions = models.data();
    scene.numberOfModels = models.size();

    scene.materialDescriptons = materials.data();
    scene.numberOfMaterials = materials.size();

    std::vector<GameObjectNode> gameObjects;

    scene.gameObjects = gameObjects.data();
    scene.numberOfGameObjects = gameObjects.size();
    m_Scene_PathTrac = new Scene(&scene, m_MaterialSystem, m_GeometrySystem, m_Device);

    m_PathTracer = new PathTracer(WINDOW_WIDTH, WINDOW_HEIGHT, m_Device);

    // m_IsInitialized = true;
}

void Application::startRasterizer()
{
    SceneDescription scene{};
    scene.name = "Resterizer Test Scene";
    std::vector<ModelDescription> models;
    std::vector<MaterialDescription> materials;

    m_State = State::Rasterizer;
    int b1StartPartId = models.size() + 1;
    int b1StartMaterialId = materials.size() + 1;
    std::string B1BattleDroidResourceFolder = "B1BattleDroid";
    getB1BattleDroidParts(b1StartPartId, B1BattleDroidResourceFolder, models);
    getB1BattleDroidMaterials(b1StartMaterialId, B1BattleDroidResourceFolder, materials);

    int buzzDroidStartPartId = models.size() + 1;
    int buzzDroidStartMaterialId = materials.size() + 1;
    std::string BuzzDroidResourceFolder = "BuzzDroid";
    getBuzzDroidParts(buzzDroidStartPartId, BuzzDroidResourceFolder, models);
    getBuzzDroidMaterials(buzzDroidStartMaterialId, BuzzDroidResourceFolder, materials);

    int jediStarfighterStartPartId = models.size() + 1;
    int jediStarfighterStartMaterialId = materials.size() + 1;
    std::string jediStarfighterResourceFolder = "JediStarfighter";
    getJediStarfighterParts(jediStarfighterStartPartId, jediStarfighterResourceFolder, models);
    getJediStarfighterMaterials(jediStarfighterStartMaterialId, jediStarfighterResourceFolder, materials);

    int tantiveIvStartPartId = models.size() + 1;
    int tantiveIvStartMaterialId = materials.size() + 1;
    std::string TantiveIvResourceFolder = "TantiveIV";
    getTantiveIVParts(tantiveIvStartPartId, TantiveIvResourceFolder, models);
    getTantiveIVMaterials(tantiveIvStartMaterialId, TantiveIvResourceFolder, materials);

    int thrantaStartPartId = models.size() + 1;
    int thrantaStartMaterialId = materials.size() + 1;
    std::string ThrantaResourceFolder = "Thranta";
    getThrantaParts(thrantaStartPartId, ThrantaResourceFolder, models);
    getThrantaMaterials(thrantaStartMaterialId, ThrantaResourceFolder, materials);

    int pongKrellStartPartId = models.size() + 1;
    int pongKrellStartMaterialId = materials.size() + 1;
    std::string PongKrellResourceFolder = "PongKrell";
    getPongKrellParts(pongKrellStartPartId, PongKrellResourceFolder, models);
    getPongKrellMaterials(pongKrellStartMaterialId, PongKrellResourceFolder, materials);

    int planePartId = models.size() + 1;
    int planeMaterialId = materials.size() + 1;
    std::string PlaneResourceFolder = "Plane";
    getPlaneParts(planePartId, PlaneResourceFolder, models);
    // Custom material for the plane
    {
        MaterialDescription desc{};
        desc.name = "Plane Material";
        desc.id = planeMaterialId;
        desc.albedo = glm::vec4(0.1f, 0.6f, 0.3f, 1.f);
        desc.albedoPipeline = new NoisePipeline(m_Device);
        desc.roughness = 0.f;
        desc.metallic = 0.f;
        desc.ao = 0.01f;
        materials.push_back(desc);
    }

    scene.modelDescriptions = models.data();
    scene.numberOfModels = models.size();

    scene.materialDescriptons = materials.data();
    scene.numberOfMaterials = materials.size();

    std::vector<GameObjectNode> gameObjects;

    uint32_t nodeId = 1;
    GameObjectNode b1BattleDroid = getB1BattleDroidParentNode(nodeId, b1StartPartId, b1StartMaterialId, "B1 Battle Droid",
                                                              glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
    // gameObjects.push_back(b1BattleDroid);

    GameObjectNode jediStarfighter = getJediStarfighterParentNode(nodeId, jediStarfighterStartPartId, jediStarfighterStartMaterialId,
                                                                  "Jedi Starfighter", glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
    // gameObjects.push_back(jediStarfighter);

    GameObjectNode buzzDroid = getBuzzDroidParentNode(nodeId, buzzDroidStartPartId, buzzDroidStartMaterialId, "Buzz Droid",
                                                      glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
    // gameObjects.push_back(buzzDroid);

    GameObjectNode tantiveIV = getTantiveIVParentNode(nodeId, tantiveIvStartPartId, tantiveIvStartMaterialId, "TantiveIV",
                                                      glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
    // gameObjects.push_back(tantiveIV);

    GameObjectNode thranta = getThrantaParentNode(nodeId, thrantaStartPartId, thrantaStartMaterialId, "Thranta",
                                                  glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
    // gameObjects.push_back(thranta);

    GameObjectNode pongKrell = getPongKrellParentNode(nodeId, pongKrellStartPartId, pongKrellStartMaterialId, "Pong Krell",
                                                      glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));

    GameObjectNode plane = getPlaneParentNode(nodeId, planePartId, planeMaterialId, "Ground Plane", glm::vec3(0.f), glm::vec3(10.f), glm::vec3(0.f));


    int xFactor = 10;
    int zFactor = 10;

    glm::vec3 center = glm::vec3(-10.f, 0.f, -5.f);
    // A group of battle droids

    int numBattleDroids = 0;
    for (int x = -xFactor/2; x < xFactor/2; ++x)
    {
        for (int z = -zFactor/2; z < zFactor/2; ++z)
        {
            GameObjectNode node = b1BattleDroid;
            node.id = nodeId++;
            node.name = "B1 Battle Droid (" + std::to_string(numBattleDroids++) + ")";
            node.position = glm::vec3(float(x), 0.f, -float(z)) + center;
            node.scale = glm::vec3(1.f);
            node.rotation = glm::vec3(0.f);
            
            gameObjects.push_back(node);
            
        }
    }

    center = glm::vec3(10.f, 0.f, -5.f);
    // A group of battle droids
    for (int x = -xFactor/2; x < xFactor/2; ++x)
    {
        for (int z = -zFactor/2; z < zFactor/2; ++z)
        {
            GameObjectNode node = b1BattleDroid;
            node.id = nodeId++;
            node.name = "B1 Battle Droid (" + std::to_string(numBattleDroids++) + ")";
            node.position = glm::vec3(float(x), 0.f, -float(z)) + center;
            node.scale = glm::vec3(1.f);
            node.rotation = glm::vec3(0.f);
            
            gameObjects.push_back(node);
            
        }
    }

    center = glm::vec3(-10.f, 0.f, -20.f);
    // A group of battle droids
    for (int x = -xFactor/2; x < xFactor/2; ++x)
    {
        for (int z = -zFactor/2; z < zFactor/2; ++z)
        {
            GameObjectNode node = b1BattleDroid;
            node.id = nodeId++;
            node.name = "B1 Battle Droid (" + std::to_string(numBattleDroids++) + ")";
            node.position = glm::vec3(float(x), 0.f, -float(z)) + center;
            node.scale = glm::vec3(1.f);
            node.rotation = glm::vec3(0.f);
            
            gameObjects.push_back(node);
            
        }
    }

    center = glm::vec3(10.f, 0.f, -20.f);
    // A group of battle droids
    for (int x = -xFactor/2; x < xFactor/2; ++x)
    {
        for (int z = -zFactor/2; z < zFactor/2; ++z)
        {
            GameObjectNode node = b1BattleDroid;
            node.id = nodeId++;
            node.name = "B1 Battle Droid (" + std::to_string(numBattleDroids++) + ")";
            node.position = glm::vec3(float(x), 0.f, -float(z)) + center;
            node.scale = glm::vec3(1.f);
            node.rotation = glm::vec3(0.f);
            
            gameObjects.push_back(node);
            
        }
    }

    center = glm::vec3(-22.f, 0.f, -5.f);
    // A group of battle droids
    for (int x = -xFactor/2; x < xFactor/2; ++x)
    {
        for (int z = -zFactor/2; z < zFactor/2; ++z)
        {
            GameObjectNode node = b1BattleDroid;
            node.id = nodeId++;
            node.name = "B1 Battle Droid (" + std::to_string(numBattleDroids++) + ")";
            node.position = glm::vec3(float(x), 0.f, -float(z)) + center;
            node.scale = glm::vec3(1.f);
            node.rotation = glm::vec3(0.f);
            
            gameObjects.push_back(node);
            
        }
    }

    center = glm::vec3(-22.f, 0.f, -20.f);
    // A group of battle droids
    for (int x = -xFactor/2; x < xFactor/2; ++x)
    {
        for (int z = -zFactor/2; z < zFactor/2; ++z)
        {
            GameObjectNode node = b1BattleDroid;
            node.id = nodeId++;
            node.name = "B1 Battle Droid (" + std::to_string(numBattleDroids++) + ")";
            node.position = glm::vec3(float(x), 0.f, -float(z)) + center;
            node.scale = glm::vec3(1.f);
            node.rotation = glm::vec3(0.f);
            
            gameObjects.push_back(node);
            
        }
    }

    center = glm::vec3(22.f, 0.f, -5.f);
    // A group of battle droids
    for (int x = -xFactor/2; x < xFactor/2; ++x)
    {
        for (int z = -zFactor/2; z < zFactor/2; ++z)
        {
            GameObjectNode node = b1BattleDroid;
            node.id = nodeId++;
            node.name = "B1 Battle Droid (" + std::to_string(numBattleDroids++) + ")";
            node.position = glm::vec3(float(x), 0.f, -float(z)) + center;
            node.scale = glm::vec3(1.f);
            node.rotation = glm::vec3(0.f);
            
            gameObjects.push_back(node);
            
        }
    }

    center = glm::vec3(22.f, 0.f, -20.f);
    // A group of battle droids
    for (int x = -xFactor/2; x < xFactor/2; ++x)
    {
        for (int z = -zFactor/2; z < zFactor/2; ++z)
        {
            GameObjectNode node = b1BattleDroid;
            node.id = nodeId++;
            node.name = "B1 Battle Droid (" + std::to_string(numBattleDroids++) + ")";
            node.position = glm::vec3(float(x), 0.f, -float(z)) + center;
            node.scale = glm::vec3(1.f);
            node.rotation = glm::vec3(0.f);
            
            gameObjects.push_back(node);
            
        }
    }

    {
        // Jedi Starfighter
        GameObjectNode node = jediStarfighter;
        node.id = nodeId++;
        node.name = "Jedi Starfighter";
        node.position = glm::vec3(0.0f, 4.f, -5.f);
        node.scale = glm::vec3(1.f);
        node.rotation = glm::vec3(glm::radians(33.f), 0.f, 0.f);
        gameObjects.push_back(node);
    }

    {
        // Thranta class destroyer
        GameObjectNode node = thranta;
        node.id = nodeId++;
        node.name = "Thranta Class Destroyer (1)";
        node.position = glm::vec3(-40.0f, 20.f, -15.f);
        node.scale = glm::vec3(1.f);
        node.rotation = glm::vec3(0.f);//, glm::radians(90.f), 0.f);
        gameObjects.push_back(node);

        node.id = nodeId++;
        node.name = "Thranta Class Destroyer (2)";
        node.position = glm::vec3(40.0f, 20.f, -15.f);
        node.scale = glm::vec3(1.f);
        node.rotation = glm::vec3(0.f);//, glm::radians(90.f), 0.f);
        gameObjects.push_back(node);
    }

    {
        // Buzz Droid
        GameObjectNode node = buzzDroid;
        node.id = nodeId++;
        node.name = "Buzz Droid";
        node.position = glm::vec3(0.0f, 0.0f, 2.f);
        node.scale = glm::vec3(1.0f);
        node.rotation = glm::vec3(0, 0.f, 0.f);
        gameObjects.push_back(node);
    }

    {
        // TantiveIV
        GameObjectNode node = tantiveIV;
        node.id = nodeId++;
        node.name = "TantiveIV";
        node.position = glm::vec3(0.0f, 20.0f, -75.f);
        node.scale = glm::vec3(5.0f);
        node.rotation = glm::vec3(0, glm::radians(90.f), 0.f);
        gameObjects.push_back(node);
    }

    {
        // Pong Krell
        GameObjectNode node = pongKrell;
        node.id = nodeId++;
        node.name = "Pong Krell";
        node.position = glm::vec3(0.0f, 0.0f, 10.f);
        node.scale = glm::vec3(1.0f);
        node.rotation = glm::vec3(0, glm::radians(180.f), 0.f);
        gameObjects.push_back(node);
    }

    {
        //Ground plane
        GameObjectNode node = plane;
        node.id = nodeId++;
        node.name = "Plane";
        node.position = glm::vec3(0.f);
        node.scale = glm::vec3(100.f);
        node.rotation = glm::vec3(0.f);
        gameObjects.push_back(node);
    }

    scene.gameObjects = gameObjects.data();
    scene.numberOfGameObjects = gameObjects.size();
    m_Scene_Raster = new Scene(&scene, m_MaterialSystem, m_GeometrySystem, m_Device);
    m_ActiveScene = m_Scene_Raster;
    m_Renderer = new Renderer(WINDOW_WIDTH, WINDOW_HEIGHT, m_Device);

    m_IsInitialized = true;
}

Application *Application::get()
{
    return s_Instance;
}