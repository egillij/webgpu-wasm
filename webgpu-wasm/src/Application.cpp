#include "Application.h"

#include "Scene/Scene.h"
#include "Scene/SceneUtils/B1BattleDroid.h"

#include "Renderer/Renderer.h"
#include "Renderer/WebGPU/wgpuDevice.h"

#include "Renderer/MaterialSystem.h"
#include "Renderer/TextureSystem.h"
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
    m_TextureSystem = new TextureSystem(m_Device);

    const char *battleDroidFile = "b1_battle_droid.obj"; //"character.obj";

    SceneDescription scene{};
    scene.name = "Test Scene";
    std::vector<ModelDescription> models;
    int b1StartPartId = 1;
    getB1BattleDroidParts(b1StartPartId, models);
    printf("Model size after add: %zu\n", models.size());
    // {
    //     // Battle droid
    //     {
    //         ModelDescription model{};
    //         model.id = 1;
    //         model.name = "Battle Droid";
    //         model.filename = "Mesh51.001_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    //     {
    //         ModelDescription model{};
    //         model.id = 2;
    //         model.name = "Blaster";
    //         model.filename = "Mesh51.001_1.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    // }

    // {
    //     // Jedi Starfighter
    //     {
    //         ModelDescription model{};
    //         model.id = 3;
    //         model.name = "Starfighter_main";
    //         model.filename = "jedi_starfighter/main_mainShape_jsf_mainMat_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    //     {
    //         ModelDescription model{};
    //         model.id = 4;
    //         model.name = "Starfighter_aux";
    //         model.filename = "jedi_starfighter/aux_auxShape_jsf_auxMat_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    // }

    // {
    //     // Thranta class destroyer
    //     {
    //         ModelDescription model{};
    //         model.id = 5;
    //         model.name = "thranta_metal02";
    //         model.filename = "thranta/Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_metal02_d_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    //     {
    //         ModelDescription model{};
    //         model.id = 6;
    //         model.name = "thranta_detail";
    //         model.filename = "thranta/Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_detail_d_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    //     {
    //         ModelDescription model{};
    //         model.id = 7;
    //         model.name = "thranta_metal01";
    //         model.filename = "thranta/Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_metal01_d_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    //     {
    //         ModelDescription model{};
    //         model.id = 8;
    //         model.name = "thranta_engine";
    //         model.filename = "thranta/Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_engine_d_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    //     {
    //         ModelDescription model{};
    //         model.id = 9;
    //         model.name = "thranta_trim";
    //         model.filename = "thranta/Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_trim_d_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    // }

    // {
    //     // Buzz droid
    //     {
    //         ModelDescription model{};
    //         model.id = 10;
    //         model.name = "buzz_droid_upper";
    //         model.filename = "buzz_droid/Box038_meshId0_name_M_Buzz_UpperBody_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    //     {
    //         ModelDescription model{};
    //         model.id = 11;
    //         model.name = "buzz_droid_lower";
    //         model.filename = "buzz_droid/Box038.001_meshId1_name_M_Buzz_LowerBody_0.geom";
    //         model.position = glm::vec3(0.f);
    //         model.scale = glm::vec3(1.f);
    //         model.rotation = glm::vec3(0.f);
    //         models.push_back(model);
    //     }
    // }

    std::vector<MaterialDescription> materials;
    int b1StartMaterialId = 1;
    getB1BattleDroidMaterials(b1StartMaterialId, materials);
    // {
    //     MaterialDescription material{};
    //     material.id = 1;
    //     material.name = "Droid Material";
    //     material.filename = "01___Def.mats";
    //     material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 2;
    //     material.name = "Blaster Material";
    //     material.filename = "07___Def.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 3;
    //     material.name = "Starfighter main mat";
    //     material.filename = "jedi_starfighter/jsf_mainMat.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 4;
    //     material.name = "Starfighter aux mat";
    //     material.filename = "jedi_starfighter/jsf_auxMat.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 5;
    //     material.name = "Thranta metal02";
    //     material.filename = "thranta/veh_rep_destroyer_metal02_d.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 6;
    //     material.name = "Thranta detail";
    //     material.filename = "thranta/veh_rep_destroyer_detail_d.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 7;
    //     material.name = "Thranta metal01";
    //     material.filename = "thranta/veh_rep_destroyer_metal01_d.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 8;
    //     material.name = "Thranta engine";
    //     material.filename = "thranta/veh_rep_destroyer_engine_d.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 9;
    //     material.name = "Thranta trim";
    //     material.filename = "thranta/veh_rep_destroyer_trim_d.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 10;
    //     material.name = "M_Buzz_UpperBody";
    //     material.filename = "buzz_droid/M_Buzz_UpperBody.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }
    // {
    //     MaterialDescription material{};
    //     material.id = 11;
    //     material.name = "M_Buzz_LowerBody";
    //     material.filename = "buzz_droid/M_Buzz_LowerBody.mats";
    //     material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
    //     materials.push_back(material);
    // }

    scene.modelDescriptions = models.data();
    scene.numberOfModels = models.size();

    scene.materialDescriptons = materials.data();
    scene.numberOfMaterials = materials.size();

    std::vector<GameObjectNode> gameObjects;

    uint32_t nodeId = 1;
    GameObjectNode b1BattleDroid = getB1BattleDroidParentNode(nodeId, b1StartPartId, b1StartMaterialId, "B1 Battle Droid", glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
    gameObjects.push_back(b1BattleDroid);

    // int xFactor = 10;
    // int zFactor = 10;
    // float xOffset = float(xFactor) / 2.f;
    // float zOffset = float(zFactor) / 2.f;

    

    // // A group of battle droids
    // for (int x = 0; x < xFactor; ++x)
    // {
    //     for (int z = 0; z < zFactor; ++z)
    //     {
    //         GameObjectNode node{};
    //         node.id = nodeId++;
    //         node.name = "B1 Battle Droid (" + std::to_string(x*zFactor+z) + ")";
    //         node.position = glm::vec3(float(x) - xOffset, 0.f, -float(z));
    //         node.scale = glm::vec3(1.f);
    //         node.rotation = glm::vec3(0.f);
    //         {
    //             GameObjectNode object{};
    //             object.id = nodeId++;
    //             object.name = "Droid " + std::to_string(x*zFactor+z);
    //             object.modelId = 1;
    //             object.materialId = 1;
    //             object.position = glm::vec3(0.f);
    //             object.scale = glm::vec3(1.f);
    //             object.rotation = glm::vec3(0.f);
    //             node.children.push_back(object);
    //         }
            
    //         {
    //             GameObjectNode object{};
    //             object.id = nodeId++;
    //             object.name = "Blaster " + std::to_string(x*zFactor+z);
    //             object.modelId = 2;
    //             object.materialId = 2;
    //             object.position = glm::vec3(0.f);
    //             object.scale = glm::vec3(1.f);
    //             object.rotation = glm::vec3(0.f);
    //             node.children.push_back(object);
    //         }

    //         gameObjects.push_back(node);
            
    //     }
    // }

    // {
    //     // Jedi Starfighter
    //     GameObjectNode node{};
    //     node.id = nodeId++;
    //     node.name = "Jedi Starfighter";
    //     node.position = glm::vec3(0.0f, 4.f, -5.f);
    //     node.scale = glm::vec3(1.f);
    //     node.rotation = glm::vec3(glm::radians(33.f), 0.f, 0.f);
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "starfighter_main";
    //         object.modelId = 3;
    //         object.materialId = 3;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(1.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }
        
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "starfighter_aux";
    //         object.modelId = 4;
    //         object.materialId = 4;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(1.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }

    //     gameObjects.push_back(node);
    // }

    // {
    //     // Thranta class destroyer
    //     GameObjectNode node{};
    //     node.id = nodeId++;
    //     node.name = "Thranta Class Destroyer";
    //     node.position = glm::vec3(0.0f, 10.f, -50.f);
    //     node.scale = glm::vec3(1.f);
    //     node.rotation = glm::vec3(0.f, glm::radians(90.f), 0.f);
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "thranta_metal02";
    //         object.modelId = 5;
    //         object.materialId = 5;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(1.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "thranta_detail";
    //         object.modelId = 6;
    //         object.materialId = 6;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(1.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "thranta_metal01";
    //         object.modelId = 7;
    //         object.materialId = 7;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(0.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "thranta_engine";
    //         object.modelId = 8;
    //         object.materialId = 8;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(1.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "thranta_trim";
    //         object.modelId = 9;
    //         object.materialId = 9;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(1.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }

    //     gameObjects.push_back(node);
    // }

    // {
    //     // Buzz Droid
    //     GameObjectNode node{};
    //     node.id = nodeId++;
    //     node.name = "Buzz Droid";
    //     node.position = glm::vec3(0.0f, 0.0f, 2.f);
    //     node.scale = glm::vec3(1.0f);
    //     node.rotation = glm::vec3(0, 0.f, 0.f);
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "buzz_droid_upper";
    //         object.modelId = 10;
    //         object.materialId = 10;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(1.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }
        
    //     {
    //         GameObjectNode object{};
    //         object.id = nodeId++;
    //         object.name = "buzz_droid_lower";
    //         object.modelId = 11;
    //         object.materialId = 11;
    //         object.position = glm::vec3(0.f);
    //         object.scale = glm::vec3(1.f);
    //         object.rotation = glm::vec3(0.f);
    //         node.children.push_back(object);
    //     }

    //     gameObjects.push_back(node);
    // }

    scene.gameObjects = gameObjects.data();
    scene.numberOfGameObjects = gameObjects.size();



    m_Scene = new Scene(&scene, m_MaterialSystem, m_GeometrySystem, m_Device);
    m_Renderer = new Renderer(WINDOW_WIDTH, WINDOW_HEIGHT, m_Device);

    m_IsInitialized = true;

    onUpdate();
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