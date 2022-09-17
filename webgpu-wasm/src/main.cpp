
#include <webgpu/webgpu_cpp.h>

#include <cstdio>
#include <memory>

#include <emscripten.h>
// #include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

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

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


// static wgpu::Device device;
// static wgpu::Queue queue;
// static wgpu::Buffer readbackBuffer;
// static wgpu::RenderPipeline pipeline;
// static wgpu::SwapChain swapChain;
// static wgpu::Buffer vertexBuffer;
// static wgpu::Buffer indexBuffer;
// static wgpu::Buffer uniformBuffer;

// static wgpu::BindGroup sceneUniformBindGroup;

static WGpuDevice wDevice;
static WGpuSwapChain wSwapChain;
static WGpuVertexBuffer vertexBuffer;
static WGpuIndexBuffer indexBuffer;
static WGpuUniformBuffer uniformBuffer;

static WGpuShader* shader = nullptr;

static WGpuBindGroup* sceneUniformBindGroup = nullptr;

static WGpuPipeline* pipeline = nullptr;

// static wgpu::Texture testTexture;
static WGpuTexture* texture;
static WGpuSampler* sampler;

struct Uniforms {
    glm::vec4 color;
};

static float triangleVertices[3*5] = {-0.5f, -0.5f, 0.f, 0.0f, 0.0f,
                                    0.5, -0.5, 0.f, 1.0f, 0.0f,
                                    0.f, 0.5f, 0.f, 0.5f, 1.0f};
static uint32_t triangleIndices[3] = {0, 1, 2};

static float cubeVertices[8*3] = {-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 0.5f, -0.5f,  -0.5f, 0.5f, -0.5f,
                                  -0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f};
static uint32_t cubeIndices[3*8] = {0, 1 ,2, 0, 2, 3,  0, 3, 7, 0, 7, 4,   4, 5, 6, 4, 6, 7,  1, 5, 6, 1, 6, 2}; // Missing the top and bottom faces


static const char shaderCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) fragUV : vec2<f32>,
    }

    @vertex
    fn main_v(
        @location(0) positionIn : vec3<f32>,
        @location(1) uvIn : vec2<f32>
    ) -> VertexOutput {
        var output : VertexOutput;
        output.position = vec4<f32>(positionIn, 1.0);
        output.fragUV = uvIn;
        return output;
    }

    struct ColorUni {
        color : vec4<f32>
    };

    @group(0) @binding(0) var<uniform> colorU : ColorUni;
    @group(0) @binding(1) var mySampler : sampler;
    @group(0) @binding(2) var myTexture : texture_2d<f32>;

    @fragment
    fn main_f(
        @location(0) fragUV : vec2<f32>
    ) -> @location(0) vec4<f32> {
        let fragColor : vec4<f32> = textureSample(myTexture, mySampler, fragUV);
        return fragColor;
    }
)";

void GetDevice(void (*callback)(wgpu::Device)){
    const WGPUInstance instance = nullptr;

    WGPURequestAdapterOptions options;
    options.powerPreference = WGPUPowerPreference::WGPUPowerPreference_HighPerformance;
    
    wgpuInstanceRequestAdapter(instance, &options, [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * userdata) {

        if(status != WGPURequestAdapterStatus::WGPURequestAdapterStatus_Success){
            printf("Failed to get adapter: %s\n", message);
        }
        else {
            WGPUAdapterProperties props;
            wgpuAdapterGetProperties(adapter, &props);

            printf("Adapter properties:\n");
            printf("\tName: %s\n", (props.name ? props.name : ""));
            printf("\tDriver description: %s\n", (props.driverDescription ? props.driverDescription : ""));
            printf("\tVendor id: %u\n", props.vendorID);
            printf("\tDevice id: %u\n", props.deviceID);
            printf("\tAdapter Type: %i\n", static_cast<int>(props.adapterType));
            printf("\tBackend Type: %i\n", static_cast<int>(props.backendType));
            
            WGPUDeviceDescriptor deviceDescription;
            deviceDescription.label = "Graphics device";

            wgpuAdapterRequestDevice(adapter, &deviceDescription, [](WGPURequestDeviceStatus status, WGPUDevice dev, char const * message, void * userdata){
                if(status != WGPURequestDeviceStatus_Success){
                    printf("Failed to get device from adapter: %s\n", message);
                }
                else {
                    wgpu::Device device_ = wgpu::Device::Acquire(dev);
                    reinterpret_cast<void (*)(wgpu::Device)>(userdata)(device_);
                }
            }, userdata);
            
        }
    }, reinterpret_cast<void*>(callback));
}

void init() {
    shader = new WGpuShader("Color Shader", shaderCode, &wDevice);

    uniformBuffer = WGpuUniformBuffer(&wDevice, "Uniform Buffer", sizeof(Uniforms));

    // Load texture
    emscripten_wget("/webgpu-wasm/avatar.jpg", "./avatar.jpg");
    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    unsigned char* imageData = stbi_load("./avatar.jpg", &width, &height, &channels, 4);

    // wgpu::TextureDescriptor texDesc{};
    // texDesc.label = "Test texture";
    // texDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::Extent3D texExtent{};
    texExtent.width = width;
    texExtent.height = height;
    // texDesc.size = texExtent;
    // texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;

    TextureCreateInfo texInfo{};
    texInfo.format = TextureFormat::BGRA8Unorm;
    texInfo.width = width;
    texInfo.height = height;
    texInfo.usage = {TextureUsage::CopyDst, TextureUsage::TextureBinding};

    texture = new WGpuTexture("Test texture", &texInfo, &wDevice);

    SamplerCreateInfo samplerInfo{};
    sampler = new WGpuSampler("Sampler", &samplerInfo, &wDevice);
    // testTexture = wDevice.getHandle().CreateTexture(&texDesc);

    wgpu::Queue queue = wDevice.getHandle().GetQueue();
    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = texture->getHandle();
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = width * 4;
    texDataLayout.rowsPerImage = height;
    texDataLayout.offset = 0;

    queue.WriteTexture(&imgCpyTex, imageData, width*height*4, &texDataLayout, &texExtent);


    sceneUniformBindGroup = new WGpuBindGroup("Scene Uniform Bind Group");
    sceneUniformBindGroup->addBuffer(&uniformBuffer, BufferBindingType::Uniform, uniformBuffer.getSize(), 0, wgpu::ShaderStage::Fragment);
    sceneUniformBindGroup->addSampler(sampler, SamplerBindingType::Filtering, 1, wgpu::ShaderStage::Fragment);
    sceneUniformBindGroup->addTexture(texture, TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    
    sceneUniformBindGroup->build(&wDevice);

    pipeline = new WGpuPipeline();
    pipeline->addBindGroup(sceneUniformBindGroup);
    pipeline->setShader(shader);
    pipeline->build(&wDevice);

    vertexBuffer = WGpuVertexBuffer(&wDevice, "Vertex Buffer", triangleVertices, 3*5*sizeof(float));
    indexBuffer = WGpuIndexBuffer(&wDevice, "Index Buffer", triangleIndices, 3*sizeof(uint32_t), IndexBufferFormat::UNSIGNED_INT_32);

}

void render() {
    wgpu::Queue queue = wDevice.getHandle().GetQueue();
    
    wgpu::TextureView backBuffer = wSwapChain.getCurrentFrameTexture();// swapChain.GetCurrentTextureView();

    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = backBuffer;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    attachment.clearValue = {0, 0, 0, 1};

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 1;
    renderPassDescription.colorAttachments = &attachment;
    
    static Uniforms uniformColor;
    static float t = 0.0f;
    float weight = glm::abs(glm::sin(t*glm::pi<float>()/10.f));
    uniformColor.color = glm::vec4(1.f, 0.502f, 0.f, 1.f) * weight + glm::vec4(0.f, 0.498f, 1.f, 1.f) * (1.f-weight);

    // printf("Frame %f\n", weight);

    t += 0.1f;
    // uniformColor.color[0] = 1.0f;
    // uniformColor.color[1] = 0.502f;
    // uniformColor.color[2] = 0.f;
    // uniformColor.color[3] = 1.f;

    queue.WriteBuffer(uniformBuffer.getHandle(), 0, (void*) glm::value_ptr<float>(uniformColor.color), sizeof(Uniforms));

    static uint64_t frameNr = 0;

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = wDevice.getHandle().CreateCommandEncoder();
        {   
            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(pipeline->getPipeline());
            renderPass.SetVertexBuffer(0, vertexBuffer.getHandle());
            renderPass.SetIndexBuffer(indexBuffer.getHandle(), static_cast<wgpu::IndexFormat>(indexBuffer.getDataFormat()));
            renderPass.SetBindGroup(0, sceneUniformBindGroup->get());
            renderPass.DrawIndexed(3);
            renderPass.End();
            
        }
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);
    frameNr++;
}

void run() {
    printf("Running\n");

    // Create swapchain
    wSwapChain = WGpuSwapChain(&wDevice, 800, 600);

    // Start main loop
    emscripten_set_main_loop(render, 0, true);
}


int main()
{
    GetDevice([](wgpu::Device device_){
        wDevice = WGpuDevice(device_);
        init();
        run();       
    }); 

}