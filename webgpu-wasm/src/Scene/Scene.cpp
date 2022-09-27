#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"

#include <webgpu/webgpu_cpp.h>
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuPipeline.h"

#include <emscripten/emscripten.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

///////////////////////////////////////////////////////////////////
// Float3 position, Float3 normal, Float2 uv
static float cubeVertices[6*6*8] = {
    1.f, -1.f, 1.f,   0.f, -1.f, 0.f,    1.f, 1.f,
  -1.f, -1.f, 1.f,    0.f, -1.f, 0.f,    0.f, 1.f,
  -1.f, -1.f, -1.f,   0.f, -1.f, 0.f,    0.f, 0.f,
  1.f, -1.f, -1.f,    0.f, -1.f, 0.f,    1.f, 0.f,
  1.f, -1.f, 1.f,     0.f, -1.f, 0.f,    1.f, 1.f,
  -1.f, -1.f, -1.f,   0.f, -1.f, 0.f,    0.f, 0.f,

  1.f, 1.f, 1.f,      1.f, 0.f, 0.f,    1.f, 1.f,
  1.f, -1.f, 1.f,     1.f, 0.f, 0.f,    0.f, 1.f,
  1.f, -1.f, -1.f,    1.f, 0.f, 0.f,    0.f, 0.f,
  1.f, 1.f, -1.f,     1.f, 0.f, 0.f,    1.f, 0.f,
  1.f, 1.f, 1.f,      1.f, 0.f, 0.f,    1.f, 1.f,
  1.f, -1.f, -1.f,    1.f, 0.f, 0.f,    0.f, 0.f,

  -1.f, 1.f, 1.f,     0.f, 1.f, 0.f,    1.f, 1.f,
  1.f, 1.f, 1.f,      0.f, 1.f, 0.f,    0.f, 1.f,
  1.f, 1.f, -1.f,     0.f, 1.f, 0.f,    0.f, 0.f,
  -1.f, 1.f, -1.f,    0.f, 1.f, 0.f,    1.f, 0.f,
  -1.f, 1.f, 1.f,     0.f, 1.f, 0.f,    1.f, 1.f,
  1.f, 1.f, -1.f,     0.f, 1.f, 0.f,    0.f, 0.f,

  -1.f, -1.f, 1.f,    -1.f, 0.f, 0.f,    1.f, 1.f,
  -1.f, 1.f, 1.f,     -1.f, 0.f, 0.f,    0.f, 1.f,
  -1.f, 1.f, -1.f,    -1.f, 0.f, 0.f,    0.f, 0.f,
  -1.f, -1.f, -1.f,   -1.f, 0.f, 0.f,    1.f, 0.f,
  -1.f, -1.f, 1.f,    -1.f, 0.f, 0.f,    1.f, 1.f,
  -1.f, 1.f, -1.f,    -1.f, 0.f, 0.f,    0.f, 0.f,

  1.f, 1.f, 1.f,      0.f, 0.f, 1.f,    1.f, 1.f,
  -1.f, 1.f, 1.f,     0.f, 0.f, 1.f,    0.f, 1.f,
  -1.f, -1.f, 1.f,    0.f, 0.f, 1.f,    0.f, 0.f,
  -1.f, -1.f, 1.f,    0.f, 0.f, 1.f,    0.f, 0.f,
  1.f, -1.f, 1.f,     0.f, 0.f, 1.f,    1.f, 0.f,
  1.f, 1.f, 1.f,      0.f, 0.f, 1.f,    1.f, 1.f,

  1.f, -1.f, -1.f,    0.f, 0.f, -1.f,    1.f, 1.f,
  -1.f, -1.f, -1.f,   0.f, 0.f, -1.f,    0.f, 1.f,
  -1.f, 1.f, -1.f,    0.f, 0.f, -1.f,    0.f, 0.f,
  1.f, 1.f, -1.f,     0.f, 0.f, -1.f,    1.f, 0.f,
  1.f, -1.f, -1.f,    0.f, 0.f, -1.f,    1.f, 1.f,
  -1.f, 1.f, -1.f,    0.f, 0.f, -1.f,    0.f, 0.f,
};

static uint64_t numCubeIndices = 6*6;
static uint32_t cubeIndices[6*6] = {
    0, 1, 2,
    3, 4, 5,

    6, 7, 8,
    9, 10, 11,

    12, 13, 14,
    15, 16, 17,

    18, 19, 20,
    21, 22, 23,

    24, 25, 26,
    27, 28, 29,

    30, 31, 32,
    33, 34, 35
};


static const char shaderCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>,
    }

    struct SceneUniforms {
        viewProjection : mat4x4<f32>,
    };

    struct ModelUniforms {
        modelMatrix : mat4x4<f32>,
    };

    @group(0) @binding(0) var<uniform> sceneUniforms : SceneUniforms;
    @group(1) @binding(0) var<uniform> modelUniforms : ModelUniforms;

    @vertex
    fn main_v(
        @location(0) positionIn : vec3<f32>,
        @location(1) normalIn : vec3<f32>,
        @location(2) uvIn : vec2<f32>
    ) -> VertexOutput {

        var output : VertexOutput;
        output.position = sceneUniforms.viewProjection * modelUniforms.modelMatrix * vec4<f32>(positionIn, 1.0);
        output.fragUV = uvIn;
        output.fragNormal = normalIn;
        return output;
    }

    @group(2) @binding(0) var mySampler : sampler;
    @group(2) @binding(1) var myTexture : texture_2d<f32>;

    @fragment
    fn main_f(
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>
    ) -> @location(0) vec4<f32> {
        var fragColor : vec4<f32> = textureSample(myTexture, mySampler, fragUV);
        let gamma : f32 = 1.0 / 2.2;
        let gammaVec : vec3<f32> = vec3<f32>(gamma, gamma, gamma);
        fragColor = vec4<f32>(pow(fragColor.rgb, gammaVec), 1.0);
        return fragColor;
    }
)";
/////////////////////////////////////////////////////////////////////////

static uint32_t WINDOW_WIDTH = 800;
static uint32_t WINDOW_HEIGHT = 600;
static float aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
static float fovY = glm::radians(45.f);

Scene::Scene(const SceneDescription* description, WGpuDevice* device)
{
    m_Name = description->name;

    // Temporary???? Þarf kannski ekki að hafa hér
    device_ = device;

    m_Camera.projectionMatrix = glm::perspective(fovY, aspect, 0.01f, 100.f);
    m_Camera.viewMatrix = glm::lookAt(glm::vec3(-5.f, 2.f, 5.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

    sceneUniformBuffer = new WGpuUniformBuffer(device, "Scene Uniform Buffer", sizeof(SceneUniforms));

    sceneUniformBindGroupLayout = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    sceneUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sceneUniformBuffer->getSize(), 0, wgpu::ShaderStage::Vertex);
    sceneUniformBindGroupLayout->build(device_);

    sceneUniformBindGroup = new WGpuBindGroup("Scene Uniform Bind Group");
    sceneUniformBindGroup->setLayout(sceneUniformBindGroupLayout);
    sceneUniformBindGroup->addBuffer(sceneUniformBuffer, BufferBindingType::Uniform, sceneUniformBuffer->getSize(), 0, wgpu::ShaderStage::Vertex);
    // sceneUniformBindGroup->addSampler(sampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    // sceneUniformBindGroup->addTexture(texture, TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    sceneUniformBindGroup->build(device_);

    for(int i = 0; i < description->numberOfModels; ++i){
        ModelDescription* model = description->modelDescriptions+i;
        GameObject* object = new GameObject(model->filename);
        glm::mat4 rotation = glm::mat4(1.f);
        rotation = glm::rotate(glm::mat4(1.f), model->rotation.z, glm::vec3(0.f, 0.f, 1.f));
        rotation = rotation * glm::rotate(glm::mat4(1.f), model->rotation.y, glm::vec3(0.f, 1.f, 0.f));
        rotation = rotation * glm::rotate(glm::mat4(1.f), model->rotation.x, glm::vec3(1.f, 0.f, 0.f));

        glm::mat4 transform = rotation * glm::translate(glm::mat4(1.f), model->position) * glm::scale(glm::mat4(1.f), model->scale);
        object->setMesh(model->filename, transform, device);

        m_GameObjects.push_back(object);
    }


    // Init renderer
    shader = new WGpuShader("Color Shader", shaderCode, device);

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
    texInfo.format = TextureFormat::RGBA8Unorm;
    texInfo.width = width;
    texInfo.height = height;
    texInfo.usage = {TextureUsage::CopyDst, TextureUsage::TextureBinding};

    texture = new WGpuTexture("Test texture", &texInfo, device);

    SamplerCreateInfo samplerInfo{};
    sampler = new WGpuSampler("Sampler", &samplerInfo, device);
    // testTexture = wDevice.getHandle().CreateTexture(&texDesc);

    wgpu::Queue queue = device->getHandle().GetQueue();
    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = texture->getHandle();
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = width * 4;
    texDataLayout.rowsPerImage = height;
    texDataLayout.offset = 0;

    queue.WriteTexture(&imgCpyTex, imageData, width*height*4, &texDataLayout, &texExtent);

    pipeline = new WGpuPipeline();



    // TODO: Ég ætti að nota bindgrouplayout hér þar sem hægt er að nota margar bindgroups með sama layout þegar við teiknum
    materialBindGroupLayout = new WGpuBindGroupLayout("Material Bind Group Layout");
    materialBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    materialBindGroupLayout->addTexture(TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    materialBindGroupLayout->build(device_);

    materialBindGroup = new WGpuBindGroup("Material Bind Group");
    materialBindGroup->setLayout(materialBindGroupLayout);
    materialBindGroup->addSampler(sampler, SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    materialBindGroup->addTexture(texture, TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    materialBindGroup->build(device_);


    modelUniformBindGroupLayout = new WGpuBindGroupLayout("Model Bind Group Layout");
    modelUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(ModelUniforms), 0, wgpu::ShaderStage::Vertex);
    modelUniformBindGroupLayout->build(device_);

    pipeline->addBindGroup(sceneUniformBindGroupLayout);
    pipeline->addBindGroup(modelUniformBindGroupLayout);
    pipeline->addBindGroup(materialBindGroupLayout);
    pipeline->setShader(shader);
    pipeline->build(device);

    wSwapChain = new WGpuSwapChain(device, WINDOW_WIDTH, WINDOW_HEIGHT);

    TextureCreateInfo depthInfo{};
    depthInfo.format = TextureFormat::Depth32Float;
    depthInfo.height = WINDOW_HEIGHT;
    depthInfo.width = WINDOW_WIDTH;
    depthInfo.usage = {TextureUsage::RenderAttachment};
    depthTexture = new WGpuTexture("Depth texture", &depthInfo, device);

// // Load the file (synchronously for now)
//     const char* battleDroidFile = "./b1BattleDroid.obj";
//     emscripten_wget("/webgpu-wasm/b1_battle_droid.obj", battleDroidFile);
//     ModelData b1BattleDroid = ModelLoader::loadModelFromFile(battleDroidFile);
//     printf("Battle Droid: %zu\n", b1BattleDroid.modelData.size());

//     b1VertexBuffer = new WGpuVertexBuffer(m_Device, "B1 Vertex Buffer", b1BattleDroid.modelData.at(0).vertexData, b1BattleDroid.modelData.at(0).numberOfVertices*8*sizeof(float));
//     b1IndexBuffer = new WGpuIndexBuffer(m_Device, "B1 Index Buffer", b1BattleDroid.modelData.at(0).indexData, b1BattleDroid.modelData.at(0).numberOfIndices, IndexBufferFormat::UNSIGNED_INT_32);
}

Scene::~Scene()
{

}

void Scene::onUpdate()
{
    printf("Scene::onUpdate()\n");
    //Render all objects
    wgpu::Queue queue = device_->getHandle().GetQueue();
    
    wgpu::TextureView backBuffer = wSwapChain->getCurrentFrameTexture();// swapChain.GetCurrentTextureView();

    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = backBuffer;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    attachment.clearValue = {0, 0, 0, 1};
 
    wgpu::RenderPassDepthStencilAttachment depthAttachment{};
    
    wgpu::TextureViewDescriptor depthViewDesc{};
    depthViewDesc.label = "Depth View",
    depthViewDesc.format = wgpu::TextureFormat::Depth32Float;
    depthViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    depthViewDesc.mipLevelCount = 1;
    depthViewDesc.arrayLayerCount = 1;

    depthAttachment.view = depthTexture->getHandle().CreateView(&depthViewDesc);
    depthAttachment.depthClearValue = 1.f;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthAttachment.depthReadOnly = false;

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 1;
    renderPassDescription.colorAttachments = &attachment;
    renderPassDescription.depthStencilAttachment = &depthAttachment;
    
    static float t = 0.0f;
    // float weight = glm::abs(glm::sin(t*glm::pi<float>()/10.f));
    // uniformColor.color = glm::vec4(1.f, 0.502f, 0.f, 1.f) * weight + glm::vec4(0.f, 0.498f, 1.f, 1.f) * (1.f-weight);
    
    static float radius = 5.f;
    static float phi = 0.f;
    static float theta = 0.f;

    float x = radius * glm::cos(phi) * glm::sin(theta);
    float y = radius * glm::sin(phi) * glm::sin(theta);
    float z = radius * glm::cos(theta);

    //TODO: make camera spin
    m_Camera.viewMatrix = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    sceneUniforms.viewProjection = m_Camera.projectionMatrix * m_Camera.viewMatrix;


    // printf("Frame %f\n", weight);

    t += 1.f/60.f;
    phi = glm::radians(10.f)*t;
    theta = glm::radians(5.f)*t;
    // uniformColor.color[0] = 1.0f;
    // uniformColor.color[1] = 0.502f;
    // uniformColor.color[2] = 0.f;
    // uniformColor.color[3] = 1.f;

    queue.WriteBuffer(sceneUniformBuffer->getHandle(), 0, (void*) &sceneUniforms, sizeof(SceneUniforms));

    static uint64_t frameNr = 0;

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device_->getHandle().CreateCommandEncoder();
        {   
            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(pipeline->getPipeline());
            renderPass.SetBindGroup(0, sceneUniformBindGroup->get());
            WGpuIndexBuffer* iBuffer = nullptr;
            WGpuVertexBuffer* vBuffer = nullptr;
            
            for(int i = 0; i < m_GameObjects.size(); ++i){
                GameObject* object = m_GameObjects.at(i);
                TriangleMesh* mesh = object->getMesh();
                if(!mesh) continue;
                
                WGpuBindGroup* modelBindGroup = object->getModelBindGroup();
                renderPass.SetVertexBuffer(0, mesh->getVertexBuffer()->getHandle());

                WGpuIndexBuffer* indexBuffer = mesh->getIndexBuffer();
                renderPass.SetIndexBuffer(indexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(indexBuffer->getDataFormat()));

                renderPass.SetBindGroup(1, modelBindGroup->get());
                renderPass.SetBindGroup(2, materialBindGroup->get());

                renderPass.DrawIndexed(indexBuffer->getIndexCount());
            }
            
            renderPass.End();
            
        }
        commands = encoder.Finish();
        frameNr++;
    }

    queue.Submit(1, &commands);
}