#include "Application.h"

#include "ModelLoader.h"

#include "Scene/Scene.h"

#include "Renderer/Renderer.h"
#include "Renderer/WebGPU/wgpuDevice.h"

#include "Renderer/MaterialSystem.h"
#include "Renderer/Geometry/GeometrySystem.h"

#include <emscripten.h>
// #include <emscripten/html5.h>
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

Application::Application(const std::string &applicationName)
{
    assert(!s_Instance);

    lastTime = emscripten_get_now();

    m_Name = applicationName;
    s_Instance = this;

    m_IsInitialized = false;
    m_Scene = nullptr;
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

    const char *battleDroidFile = "b1_battle_droid.obj"; //"character.obj";
    // emscripten_wget("/webgpu-wasm/b1_battle_droid.obj", battleDroidFile);

    SceneDescription scene{};
    scene.name = "Test Scene";
    std::vector<ModelDescription> models;

    {
        ModelDescription model{};
        model.id = 1;
        model.name = "Battle Droid";
        model.filename = "Mesh51.001_0.geom";
        model.position = glm::vec3(0.f);
        model.scale = glm::vec3(1.f);
        model.rotation = glm::vec3(0.f);
        models.push_back(model);
    }
    {
        ModelDescription model{};
        model.id = 2;
        model.name = "Blaster";
        model.filename = "Mesh51.001_1.geom";
        model.position = glm::vec3(0.f);
        model.scale = glm::vec3(1.f);
        model.rotation = glm::vec3(0.f);
        models.push_back(model);
    }

    std::vector<MaterialDescription> materials;
    {
        MaterialDescription material{};
        material.id = 1;
        material.name = "Droid Material";
        material.filename = "01___Def.mats";
        material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.f);
        materials.push_back(material);
    }
    {
        MaterialDescription material{};
        material.id = 2;
        material.name = "Blaster Material";
        material.filename = "07___Def.mats";
        material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
        materials.push_back(material);
    }


    scene.modelDescriptions = models.data();
    scene.numberOfModels = models.size();

    scene.materialDescriptons = materials.data();
    scene.numberOfMaterials = materials.size();

    std::vector<GameObjectNode> gameObjects;

    int xFactor = 50;
    int zFactor = 50;
    float xOffset = float(xFactor) / 2.f;
    float zOffset = float(zFactor) / 2.f;

    for (int x = 0; x < xFactor; ++x)
    {
        for (int z = 0; z < zFactor; ++z)
        {
            {
                GameObjectNode object{};
                object.id = x*zFactor+z;
                object.name = "Droid " + std::to_string(object.id);
                object.modelId = 1;
                object.materialId = 1;
                object.position = glm::vec3(float(x) - xOffset, 0.f, -float(z));
                object.scale = glm::vec3(1.f);
                object.rotation = glm::vec3(0.f);
                gameObjects.push_back(object);
            }
            
            {
                GameObjectNode object{};
                object.id = x*zFactor+z;
                object.name = "Blaster " + std::to_string(object.id);
                object.modelId = 2;
                object.materialId = 2;
                object.position = glm::vec3(float(x) - xOffset, 0.f, -float(z));
                object.scale = glm::vec3(1.f);
                object.rotation = glm::vec3(0.f);
                gameObjects.push_back(object);
            }
            
        }
    }
    scene.gameObjects = gameObjects.data();
    scene.numberOfGameObjects = gameObjects.size();



    m_Scene = new Scene(&scene, m_MaterialSystem, m_GeometrySystem, m_Device);
    m_Renderer = new Renderer(WINDOW_WIDTH, WINDOW_HEIGHT, m_Device);

    m_IsInitialized = true;
}

void Application::onUpdate()
{
    if (!m_IsInitialized)
        return;

    if (m_Scene)
        m_Scene->onUpdate();

    if (m_Renderer)
    {
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
            elem.innerHTML = $0;
            }, currentFPS
        );

        m_Renderer->render(m_Scene);
    }

    ++frameNo;
}

Application *Application::get()
{
    return s_Instance;
}