#include "Application.h"

#include "ModelLoader.h"

#include "Scene/Scene.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuPipeline.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"

#include <emscripten.h>
// #include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <cassert>
#include <cstdio>
#include <memory.h>

#include <glm/gtc/matrix_transform.hpp>

////////////////////////////////////////////////////////////////////////////////////////
// Temporary, will be moved/handled differently
static uint32_t WINDOW_WIDTH = 800;
static uint32_t WINDOW_HEIGHT = 600;
static float aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
static float fovY = glm::radians(45.f);



////////////////////////////////////////////////////////////////////////////////////////

static Application * s_Instance = nullptr;

static WGpuTexture* depthTexture = nullptr;

void GetDevice(){
    const WGPUInstance instance = nullptr;

    WGPURequestAdapterOptions *options = (WGPURequestAdapterOptions*)malloc(sizeof(WGPURequestAdapterOptions));
    memset(options, 0, sizeof(WGPURequestAdapterOptions));
    options->powerPreference = WGPUPowerPreference::WGPUPowerPreference_HighPerformance;
    options->forceFallbackAdapter = false;
    
    wgpuInstanceRequestAdapter(instance, options, [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * userdata) {

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
            
        }
    }, nullptr);
}

Application::Application(const std::string& applicationName) 
{
    assert(!s_Instance);

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

    const char* battleDroidFile = "b1_battle_droid.obj";
    // emscripten_wget("/webgpu-wasm/b1_battle_droid.obj", battleDroidFile);

    SceneDescription scene{};
    scene.name = "Test Scene";
    std::vector<ModelDescription> models;
    ModelDescription model1{};
    model1.filename = battleDroidFile;
    model1.position = glm::vec3(0.f);
    model1.scale = glm::vec3(1.f);
    model1.rotation = glm::vec3(0.f);
    models.push_back(model1);

    scene.modelDescriptions = models.data();
    scene.numberOfModels = models.size();

    m_Scene = new Scene(&scene, m_Device);

    m_IsInitialized = true;

}

void Application::onUpdate()
{
    if(!m_IsInitialized) return;

    if(m_Scene) m_Scene->onUpdate();
}

Application* Application::get()
{
    return s_Instance;
}