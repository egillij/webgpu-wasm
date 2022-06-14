
#include <webgpu/webgpu_cpp.h>

#include <cstdio>
#include <memory>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

static wgpu::Device device;
static wgpu::Queue queue;
static wgpu::Buffer readbackBuffer;
static wgpu::RenderPipeline pipeline;
static wgpu::SwapChain swapChain;
static wgpu::Buffer vertexBuffer;
static wgpu::Buffer indexBuffer;
static wgpu::Buffer uniformBuffer;
static wgpu::BindGroup sceneUniformBindGroup;

struct Uniforms {
    glm::vec4 color;
};

static float triangleVertices[9] = {-0.5f, -0.5f, 0.f, 0.5, -0.5, 0.f, 0.f, 0.5f, 0.f};
static uint32_t triangleIndices[3] = {0, 1, 2};

static const char shaderCode[] = R"(
    @stage(vertex)
    fn main_v(@location(0) positionIn : vec3<f32>) -> @builtin(position) vec4<f32> {
        return vec4<f32>(positionIn, 1.0);
    }

    struct ColorUni {
        color : vec4<f32>;
    };

    @group(0) @binding(0) var<uniform> colorU : ColorUni;

    @stage(fragment)
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
    device.SetUncapturedErrorCallback([](WGPUErrorType errorType, const char* message, void* userdata){
        printf("Error: %d -> %s", errorType, message);
    }, nullptr);

    queue = device.GetQueue();

    wgpu::ShaderModule shaderModule{};
    {
        wgpu::ShaderModuleWGSLDescriptor wgslDescription{};
        wgslDescription.source = shaderCode;

        wgpu::ShaderModuleDescriptor shaderDescription{};
        shaderDescription.nextInChain = &wgslDescription;
        shaderModule = device.CreateShaderModule(&shaderDescription);
    }

    {
        wgpu::BufferDescriptor ubDescription{};
        ubDescription.label = "uniform buffer";
        ubDescription.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        ubDescription.size = sizeof(Uniforms);
        ubDescription.mappedAtCreation = false;
        uniformBuffer = device.CreateBuffer(&ubDescription);

        wgpu::BindGroupLayoutDescriptor bglDescription{};
        auto bgl = device.CreateBindGroupLayout(&bglDescription);
        wgpu::BindGroupDescriptor bgDescription{};
        bgDescription.layout = bgl;
        bgDescription.entryCount = 0;
        bgDescription.entries = nullptr;

        device.CreateBindGroup(&bgDescription);
    }

    wgpu::BindGroupLayout sceneBgl;
    {
        // wgpu::BindGroupEntry colorUniformEntry{};
        // colorUniformEntry.binding = 0;
        // colorUniformEntry.buffer = uniformBuffer;

        wgpu::BufferBindingLayout scenebbl{};
        scenebbl.type = wgpu::BufferBindingType::Uniform;
        scenebbl.minBindingSize = sizeof(Uniforms);

        wgpu::BindGroupLayoutEntry sceneBglEntry{};
        sceneBglEntry.binding = 0;
        sceneBglEntry.visibility = wgpu::ShaderStage::Fragment;
        sceneBglEntry.buffer = scenebbl;
    

        wgpu::BindGroupLayoutDescriptor sceneBglDesc{};
        sceneBglDesc.label = "Scene BindGroupLayout";
        sceneBglDesc.entryCount = 1;
        sceneBglDesc.entries = &sceneBglEntry;

        sceneBgl = device.CreateBindGroupLayout(&sceneBglDesc);

        wgpu::PipelineLayoutDescriptor plDescription{};
        plDescription.bindGroupLayoutCount = 1;
        plDescription.bindGroupLayouts = &sceneBgl;
        
        wgpu::ColorTargetState colorTargetState{};
        colorTargetState.format = wgpu::TextureFormat::BGRA8Unorm;

        wgpu::FragmentState fragmentState{};
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = "main_f";
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTargetState;

        wgpu::VertexAttribute vAttribute{};
        vAttribute.format = wgpu::VertexFormat::Float32x3;
        vAttribute.offset = 0;
        vAttribute.shaderLocation = 0;
        
        wgpu::VertexBufferLayout vertexBufferLayout{};
        vertexBufferLayout.attributeCount = 1;
        vertexBufferLayout.attributes = &vAttribute;
        vertexBufferLayout.arrayStride = 3 * sizeof(float);
        vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

        wgpu::RenderPipelineDescriptor rpDescription{};
        rpDescription.layout = device.CreatePipelineLayout(&plDescription);
        rpDescription.vertex.module = shaderModule;
        rpDescription.vertex.entryPoint = "main_v";
        rpDescription.vertex.buffers = &vertexBufferLayout;
        rpDescription.vertex.bufferCount = 1;
        rpDescription.fragment = &fragmentState;
        rpDescription.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        rpDescription.primitive.frontFace = wgpu::FrontFace::CCW;
        rpDescription.primitive.cullMode = wgpu::CullMode::Back;
        
        pipeline = device.CreateRenderPipeline(&rpDescription);
    }

    {
        wgpu::BindGroupEntry colorUniformEntry{};
        colorUniformEntry.binding = 0;
        colorUniformEntry.buffer = uniformBuffer;
        colorUniformEntry.size = sizeof(Uniforms);
        
        wgpu::BindGroupDescriptor sceneBgDescription{};
        sceneBgDescription.layout = sceneBgl;// pipeline.GetBindGroupLayout(0);
        sceneBgDescription.entryCount = 1;
        sceneBgDescription.entries = &colorUniformEntry;

        sceneUniformBindGroup = device.CreateBindGroup(&sceneBgDescription);
    }

    {
        wgpu::BufferDescriptor vbDescription{};
        vbDescription.label = "vertex buffer";
        vbDescription.usage = wgpu::BufferUsage::Vertex;
        vbDescription.size = 9 * sizeof(float);
        vbDescription.mappedAtCreation = true;
        vertexBuffer = device.CreateBuffer(&vbDescription);
        memcpy(vertexBuffer.GetMappedRange(), triangleVertices, 9*sizeof(float)); // How to set buffer data with a mapped buffer
        vertexBuffer.Unmap();

        wgpu::BufferDescriptor ibDescription{};
        ibDescription.label = "index buffer";
        ibDescription.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst    ;
        ibDescription.size = 3 * sizeof(uint32_t);
        ibDescription.mappedAtCreation = false;
        indexBuffer = device.CreateBuffer(&ibDescription);
        queue.WriteBuffer(indexBuffer, 0, (void*)triangleIndices, 3*sizeof(uint32_t)); // How to write data to a buffer using the queue


    }
}

void render() {
    wgpu::TextureView backBuffer = swapChain.GetCurrentTextureView();

    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = backBuffer;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    attachment.clearColor = {0, 0, 0, 1};

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 1;
    renderPassDescription.colorAttachments = &attachment;
    
    Uniforms uniformColor;
    static float t = 0.0f;
    float weight = glm::abs(glm::sin(t*glm::pi<float>()/10.f));
    uniformColor.color = glm::vec4(1.f, 0.502f, 0.f, 1.f) * weight + glm::vec4(0.f, 0.498f, 1.f, 1.f) * (1.f-weight);

    printf("Frame %f\n", weight);

    t+= 0.1f;
    // uniformColor.color[0] = 1.0f;
    // uniformColor.color[1] = 0.502f;
    // uniformColor.color[2] = 0.f;
    // uniformColor.color[3] = 1.f;

    queue.WriteBuffer(uniformBuffer, 0, (void*) glm::value_ptr<float>(uniformColor.color), sizeof(Uniforms));

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {   

            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(pipeline);
            renderPass.SetVertexBuffer(0, vertexBuffer);
            renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
            renderPass.SetBindGroup(0, sceneUniformBindGroup);
            renderPass.DrawIndexed(3);
            renderPass.EndPass();
            
        }
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);
}

void run() {
    printf("Running\n");

    // Create swapchain
    {
        wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
        canvasDesc.selector = "#canvas";

        wgpu::SurfaceDescriptor surfDescription{};
        surfDescription.nextInChain = &canvasDesc;
        wgpu::Instance instance{};
        wgpu::Surface surface = instance.CreateSurface(&surfDescription);

        wgpu::SwapChainDescriptor swapChainDescription{};
        swapChainDescription.usage = wgpu::TextureUsage::RenderAttachment;
        swapChainDescription.format = wgpu::TextureFormat::BGRA8Unorm;
        swapChainDescription.width = 600;
        swapChainDescription.height = 800;
        swapChainDescription.presentMode = wgpu::PresentMode::Fifo;
        swapChain = device.CreateSwapChain(surface, &swapChainDescription);
    }

    emscripten_set_main_loop(render, 0, true);
}


int main()
{
    GetDevice([](wgpu::Device device_){
        device = device_;
        init();
        run();       
    }); 

}