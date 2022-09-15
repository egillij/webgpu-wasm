
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

#include "Renderer/WebGPU/wgpuBindGroup.h"


// static wgpu::Device device;
// static wgpu::Queue queue;
// static wgpu::Buffer readbackBuffer;
static wgpu::RenderPipeline pipeline;
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

struct Uniforms {
    glm::vec4 color;
};

static float triangleVertices[9] = {-0.5f, -0.5f, 0.f, 0.5, -0.5, 0.f, 0.f, 0.5f, 0.f};
static uint32_t triangleIndices[3] = {0, 1, 2};

static float cubeVertices[8*3] = {-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 0.5f, -0.5f,  -0.5f, 0.5f, -0.5f,
                                  -0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f};
static uint32_t cubeIndices[3*8] = {0, 1 ,2, 0, 2, 3,  0, 3, 7, 0, 7, 4,   4, 5, 6, 4, 6, 7,  1, 5, 6, 1, 6, 2}; // Missing the top and bottom faces


static const char shaderCode[] = R"(
    @vertex
    fn main_v(@location(0) positionIn : vec3<f32>) -> @builtin(position) vec4<f32> {
        return vec4<f32>(positionIn, 1.0);
    }

    struct ColorUni {
        color : vec4<f32>
    };

    @group(0) @binding(0) var<uniform> colorU : ColorUni;

    @fragment
    fn main_f() -> @location(0) vec4<f32> {
        return colorU.color;
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
    // wgpu::ShaderModule shaderModule{};
    // {
    //     wgpu::ShaderModuleWGSLDescriptor wgslDescription{};
    //     wgslDescription.source = shaderCode;

    //     wgpu::ShaderModuleDescriptor shaderDescription{};
    //     shaderDescription.label = "Color Shader Description";
    //     shaderDescription.nextInChain = &wgslDescription;
    //     shaderModule = wDevice.getHandle().CreateShaderModule(&shaderDescription);
    // }

    {
        uniformBuffer = WGpuUniformBuffer(&wDevice, "Uniform Buffer", sizeof(Uniforms));

        sceneUniformBindGroup = new WGpuBindGroup("Scene Uniform Bind Group");
        sceneUniformBindGroup->addEntry(&uniformBuffer, BufferBindingType::Uniform, uniformBuffer.getSize(), 0, wgpu::ShaderStage::Fragment);
        sceneUniformBindGroup->build(&wDevice);
    }

    // wgpu::BindGroupLayout sceneBgl;
    {
        // wgpu::BindGroupEntry colorUniformEntry{};
        // colorUniformEntry.binding = 0;
        // colorUniformEntry.buffer = uniformBuffer.getHandle();

        // wgpu::BufferBindingLayout scenebbl{};
        // scenebbl.type = wgpu::BufferBindingType::Uniform;
        // scenebbl.minBindingSize = sizeof(Uniforms);

        // wgpu::BindGroupLayoutEntry sceneBglEntry{};
        // sceneBglEntry.binding = 0;
        // sceneBglEntry.visibility = wgpu::ShaderStage::Fragment;
        // sceneBglEntry.buffer = scenebbl;
    

        // wgpu::BindGroupLayoutDescriptor sceneBglDesc{};
        // sceneBglDesc.label = "Scene BindGroupLayout";
        // sceneBglDesc.entryCount = 1;
        // sceneBglDesc.entries = &sceneBglEntry;

        // sceneBgl = wDevice.getHandle().CreateBindGroupLayout(&sceneBglDesc);

        // wgpu::BindGroupEntry colorUniformEntry{};
        // colorUniformEntry.binding = 0;
        // colorUniformEntry.buffer = uniformBuffer.getHandle();
        // colorUniformEntry.size = sizeof(Uniforms);
        
        // wgpu::BindGroupDescriptor sceneBgDescription{};
        // sceneBgDescription.layout = wSceneUniformBindGroup->getLayout();// pipeline.GetBindGroupLayout(0);
        // sceneBgDescription.entryCount = 1;
        // sceneBgDescription.entries = &colorUniformEntry;

        // sceneUniformBindGroup = wDevice.getHandle().CreateBindGroup(&sceneBgDescription);
    }
    {

        wgpu::PipelineLayoutDescriptor plDescription{};
        plDescription.bindGroupLayoutCount = 1;
        plDescription.bindGroupLayouts = sceneUniformBindGroup->getLayout();// sceneUniformBindGroup.getLayout();
        
        /////////////////////////////////////
        // Add to shader class and get from there
        // wgpu::ColorTargetState colorTargetState{};
        // colorTargetState.format = wgpu::TextureFormat::BGRA8Unorm;

        // wgpu::FragmentState fragmentState{};
        // fragmentState.module = shaderModule;
        // fragmentState.entryPoint = "main_f";
        // fragmentState.targetCount = 1;
        // fragmentState.targets = &colorTargetState;

        /////////////////////////////////////

        ////////////////////////
        // Add creation to vertex buffer class and get from there??? 
        // Maybe not since this is at pipeline creation, but the same pipeline can be used for multiple vertex buffers.
        // Make a special class for this instead?? or store with the pipeline class.
        wgpu::VertexAttribute vAttribute{};
        vAttribute.format = wgpu::VertexFormat::Float32x3;
        vAttribute.offset = 0;
        vAttribute.shaderLocation = 0;
        
        wgpu::VertexBufferLayout vertexBufferLayout{};
        vertexBufferLayout.attributeCount = 1;
        vertexBufferLayout.attributes = &vAttribute;
        vertexBufferLayout.arrayStride = 3 * sizeof(float);
        vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
        ////////////////////////

        wgpu::RenderPipelineDescriptor rpDescription{};
        rpDescription.layout = wDevice.getHandle().CreatePipelineLayout(&plDescription);
        rpDescription.vertex.module = shader->getModule();
        rpDescription.vertex.entryPoint = "main_v";
        rpDescription.vertex.buffers = &vertexBufferLayout;
        rpDescription.vertex.bufferCount = 1;
        rpDescription.fragment = shader->getFragmentState();
        rpDescription.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        rpDescription.primitive.frontFace = wgpu::FrontFace::CCW;
        rpDescription.primitive.cullMode = wgpu::CullMode::Back;
        
        pipeline = wDevice.getHandle().CreateRenderPipeline(&rpDescription);
    }

    {   
        // wgpu::BindGroupEntry colorUniformEntry{};
        // colorUniformEntry.binding = 0;
        // colorUniformEntry.buffer = uniformBuffer.getHandle();
        // colorUniformEntry.size = sizeof(Uniforms);
        
        // wgpu::BindGroupDescriptor sceneBgDescription{};
        // sceneBgDescription.layout = sceneBgl;// pipeline.GetBindGroupLayout(0);
        // sceneBgDescription.entryCount = 1;
        // sceneBgDescription.entries = &colorUniformEntry;

        // sceneUniformBindGroup = wDevice.getHandle().CreateBindGroup(&sceneBgDescription);

        // printf("Create uniform buffer bind group\n");
        // wSceneUniformBindGroup = WGpuBindGroup("Scene Uniform Bind Group");
        // printf("Add entry\n");
        // wSceneUniformBindGroup.addEntry(&uniformBuffer, BufferBindingType::Uniform, uniformBuffer.getSize(), 0, wgpu::ShaderStage::Fragment);
        // printf("Build\n");
        // wSceneUniformBindGroup.build(&wDevice);
    }

    {
        // wgpu::BufferDescriptor vbDescription{};
        // vbDescription.label = "vertex buffer";
        // vbDescription.usage = wgpu::BufferUsage::Vertex;
        // vbDescription.size = 9 * sizeof(float);
        // vbDescription.mappedAtCreation = true;
        // vertexBuffer = wDevice.getHandle().CreateBuffer(&vbDescription);
        // memcpy(vertexBuffer.GetMappedRange(), triangleVertices, 9*sizeof(float)); // How to set buffer data with a mapped buffer
        // vertexBuffer.Unmap();

        vertexBuffer = WGpuVertexBuffer(&wDevice, "Vertex Buffer", triangleVertices, 9*sizeof(float));
        indexBuffer = WGpuIndexBuffer(&wDevice, "Index Buffer", triangleIndices, 3*sizeof(uint32_t), IndexBufferFormat::UNSIGNED_INT_32);


        // wgpu::BufferDescriptor ibDescription{};
        // ibDescription.label = "index buffer";
        // ibDescription.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst    ;
        // ibDescription.size = 3 * sizeof(uint32_t);
        // ibDescription.mappedAtCreation = false;
        // indexBuffer = wDevice.getHandle().CreateBuffer(&ibDescription);
        // queue.WriteBuffer(indexBuffer, 0, (void*)triangleIndices, 3*sizeof(uint32_t)); // How to write data to a buffer using the queue

        


    }
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
            renderPass.SetPipeline(pipeline);
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
    {
        // wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
        // canvasDesc.selector = "#canvas";

        // wgpu::SurfaceDescriptor surfDescription{};
        // surfDescription.nextInChain = &canvasDesc;
        // wgpu::Instance instance{};
        // wgpu::Surface surface = instance.CreateSurface(&surfDescription);

        // wgpu::SwapChainDescriptor swapChainDescription{};
        // swapChainDescription.usage = wgpu::TextureUsage::RenderAttachment;
        // swapChainDescription.format = wgpu::TextureFormat::BGRA8Unorm;
        // swapChainDescription.width = 600;
        // swapChainDescription.height = 800;
        // swapChainDescription.presentMode = wgpu::PresentMode::Fifo;
        // swapChain = wDevice.getHandle().CreateSwapChain(surface, &swapChainDescription);

        wSwapChain = WGpuSwapChain(&wDevice, 800, 600);
    }

    emscripten_set_main_loop(render, 0, true);
}


int main()
{
    GetDevice([](wgpu::Device device_){
        wDevice = WGpuDevice(device_);
        init();
        run();       
    }); 
    // printf("Starting\n");
    // const WGPUInstance instance = nullptr;
    // wDevice = WGpuDevice(instance, [](){printf("Ending\n");});
    // printf("Ended\n");

}