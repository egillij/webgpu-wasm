#include "PBRRenderPipeline.h"

#include "PBRShaders.h"

#include "Renderer/Material.h"
#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuPipeline.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuSampler.h"

#include "Scene/GameObject.h"
#include "Renderer/Geometry/TriangleMesh.h"
#include "Renderer/Geometry/Part.h"

#include "Scene/Scene.h"

#include "Application.h"

#include <glm/gtx/string_cast.hpp>

#include <set>

// Tímabundið á meðan fjöldi þríhyrninga er prentaður héðan
#include <emscripten.h>
#include <emscripten/html5.h>
#ifndef __EMSCRIPTEN__
#define EM_ASM(x, y)
#endif

// #define WAIT_FOR_QUEUE 1
// #ifdef WAIT_FOR_QUEUE
// #undef WAIT_FOR_QUEUE
// #endif

static uint32_t totalTriangles = 0;
static uint32_t uniqueTriangles = 0;
static uint32_t uniqueObjects = 0;
static uint32_t uniqueParts = 0;

struct RenderParams {
    Scene* scene = nullptr;
    WGpuSwapChain* swapChain = nullptr;
    WGpuDevice* device = nullptr;
    PBRRenderPipeline* pipeline = nullptr;
};

static RenderParams params{};

int pbrAnimationFrame(double t, void* userData) {
    // params.pipeline->run(params.scene, params.device, params.swapChain);
    Application::get()->onUpdate();
    return 1;
}

void queueDoneCallback(WGPUQueueWorkDoneStatus status, void * userdata){
    if(status == WGPUQueueWorkDoneStatus_Success){
        params.pipeline->light(params.scene, params.device, params.swapChain);
        emscripten_request_animation_frame(pbrAnimationFrame, nullptr);
    }
}


PBRRenderPipeline::PBRRenderPipeline(uint32_t width, uint32_t height, WGpuDevice* device)
: RenderPipeline("PBR Render Pipeline"), m_ModelUniformBindGroupLayout(nullptr), m_MaterialBindGroupLayout(nullptr),
  m_SceneUniformBindGroupLayout(nullptr), m_SamplerBindGroupLayout(nullptr), m_RenderShader(nullptr),
  m_LightingShader(nullptr), m_RenderPipeline(nullptr), m_LightingPipeline(nullptr),
  m_GBufferBindGroupLayout(nullptr), m_GBufferBindGroup(nullptr), m_DepthTexture(nullptr), m_CacheTransforms(true)
{
    m_SceneUniformBindGroupLayout = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    m_SceneUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(SceneUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    m_SceneUniformBindGroupLayout->build(device);

    m_MaterialBindGroupLayout = new WGpuBindGroupLayout("Material Bind Group Layout");
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 4, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(device);

    m_ModelUniformBindGroupLayout = new WGpuBindGroupLayout("Model Bind Group Layout");
    m_ModelUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(ModelUniforms), 0, wgpu::ShaderStage::Vertex);
    m_ModelUniformBindGroupLayout->build(device);

    m_SamplerBindGroupLayout = new WGpuBindGroupLayout("Sampler Bind Group Layout");
    m_SamplerBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    m_SamplerBindGroupLayout->build(device);

    ShaderDescription pbrDescription{};
    pbrDescription.shaderCode = pbrRenderCode;
    pbrDescription.colorTargets = {TextureFormat::RGBA8Unorm, TextureFormat::RGBA32Float, TextureFormat::RGBA32Float};
    m_RenderShader = new WGpuShader("PBR_Render_Shader", pbrDescription, device);
    m_RenderPipeline = new WGpuPipeline("PBR Render Pipeline");
    m_RenderPipeline->addBindGroup(m_SceneUniformBindGroupLayout);
    m_RenderPipeline->addBindGroup(m_ModelUniformBindGroupLayout);
    m_RenderPipeline->addBindGroup(m_MaterialBindGroupLayout);
    m_RenderPipeline->addBindGroup(m_SamplerBindGroupLayout);
    m_RenderPipeline->setShader(m_RenderShader);
    m_RenderPipeline->build(device, true);

    // Lighting pipeline

    m_GBufferBindGroupLayout = new WGpuBindGroupLayout("PBR GBuffer Bind Group Layout");
    m_GBufferBindGroupLayout->addTexture(TextureSampleType::Float, 0, wgpu::ShaderStage::Fragment);
    m_GBufferBindGroupLayout->addTexture(TextureSampleType::UnfilterableFloat, 1, wgpu::ShaderStage::Fragment);
    m_GBufferBindGroupLayout->addTexture(TextureSampleType::UnfilterableFloat, 2, wgpu::ShaderStage::Fragment);
    m_GBufferBindGroupLayout->build(device);

    ShaderDescription pbrLightingDescription{};
    pbrLightingDescription.shaderCode = pbrLightingCode;
    pbrLightingDescription.colorTargets = {TextureFormat::BGRA8Unorm};
    m_LightingShader = new WGpuShader("PBR_Lighting_Shader", pbrLightingDescription, device);  
    m_LightingPipeline = new WGpuPipeline("PBR Lighting Pipeline");
    m_LightingPipeline->addBindGroup(m_SceneUniformBindGroupLayout);
    m_LightingPipeline->addBindGroup(m_GBufferBindGroupLayout);
    m_LightingPipeline->addBindGroup(m_SamplerBindGroupLayout);
    m_LightingPipeline->setShader(m_LightingShader);
    m_LightingPipeline->build(device, false);

    TextureCreateInfo rgbaFloatInfo{};
    rgbaFloatInfo.format = TextureFormat::RGBA32Float;
    rgbaFloatInfo.height = height;
    rgbaFloatInfo.width = width;
    rgbaFloatInfo.usage = {TextureUsage::RenderAttachment, TextureUsage::TextureBinding};

    TextureCreateInfo rgbaUnsignedInfo{};
    rgbaUnsignedInfo.format = TextureFormat::RGBA8Unorm;
    rgbaUnsignedInfo.height = height;
    rgbaUnsignedInfo.width = width;
    rgbaUnsignedInfo.usage = {TextureUsage::RenderAttachment, TextureUsage::TextureBinding};

    m_GBuffer.albedoMetallic = new WGpuTexture("GBuffer_Albedo_Metallic", &rgbaUnsignedInfo, device);
    m_GBuffer.positionRoughness = new WGpuTexture("GBuffer_Position_Roughness", &rgbaFloatInfo, device);
    m_GBuffer.normalsAo = new WGpuTexture("GBuffer_Normals_AO", &rgbaFloatInfo, device);

    m_OutputTexture = new WGpuTexture("PBR_Output_Texture", &rgbaFloatInfo, device);

    m_GBufferBindGroup = new WGpuBindGroup("PBR GBuffer Bind Group");
    m_GBufferBindGroup->setLayout(m_GBufferBindGroupLayout); // TODO: ekki besta leiðin. Auðvelt að gleyma þessu. Setja assert í bindground ef layout er ekki til staðar.
    m_GBufferBindGroup->addTexture(m_GBuffer.albedoMetallic, TextureSampleType::Float, 0, wgpu::ShaderStage::Fragment);
    m_GBufferBindGroup->addTexture(m_GBuffer.positionRoughness, TextureSampleType::UnfilterableFloat, 1, wgpu::ShaderStage::Fragment);
    m_GBufferBindGroup->addTexture(m_GBuffer.normalsAo, TextureSampleType::UnfilterableFloat, 2, wgpu::ShaderStage::Fragment);
    m_GBufferBindGroup->build(device);

    TextureCreateInfo depthInfo{};
    depthInfo.format = TextureFormat::Depth32Float;
    depthInfo.height = height;
    depthInfo.width = width;
    depthInfo.usage = {TextureUsage::RenderAttachment};
    m_DepthTexture = new WGpuTexture("Depth texture", &depthInfo, device);


    SamplerCreateInfo samplerInfo{};
    m_NearestSampler = new WGpuSampler("Sampler", &samplerInfo, device);
    m_NearestSampler2 = new WGpuSampler("Sampler2", &samplerInfo, device);

    m_SamplerBindGroupLayout = new WGpuBindGroupLayout("Sampler Bind Group Layout");
    m_SamplerBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    m_SamplerBindGroupLayout->build(device);

    m_SamplerBindGroup = new WGpuBindGroup("Sampler Bind Group");
    m_SamplerBindGroup->setLayout(m_SamplerBindGroupLayout);
    m_SamplerBindGroup->addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    m_SamplerBindGroup->build(device);

    m_SamplerBindGroup2 = new WGpuBindGroup("Sampler Bind Group2");
    m_SamplerBindGroup2->setLayout(m_SamplerBindGroupLayout);
    m_SamplerBindGroup2->addSampler(m_NearestSampler2, SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    m_SamplerBindGroup2->build(device);
}

PBRRenderPipeline::~PBRRenderPipeline()
{
    //TODO: hreinsa allt sem þarf
    if(m_RenderPipeline) delete m_RenderPipeline;
    if(m_LightingPipeline) delete m_LightingPipeline;
}

void PBRRenderPipeline::run(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain, wgpu::Queue *queue)
{
    if(!scene || !device || !swapChain) return;

    params.pipeline = this;
    params.device = device;
    params.scene = scene;
    params.swapChain = swapChain;

    wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();

    render(scene, device, swapChain, &encoder);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue->Submit(1, &commands);

    #ifdef WAIT_FOR_QUEUE
    queue->OnSubmittedWorkDone(0, queueDoneCallback, nullptr);
    #endif

    // #ifndef WAIT_FOR_QUEUE
    // light(scene, device, swapChain, &encoder);
    // #endif
    // swapChain->present();

    // wgpu::CommandBuffer commands = encoder.Finish();
    // queue->Submit(1, &commands);
}

void PBRRenderPipeline::render(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain, wgpu::CommandEncoder* encoder)
{
    wgpu::RenderPassColorAttachment gbufferAttachments[3];

    gbufferAttachments[0].view = m_GBuffer.albedoMetallic->createView();
    gbufferAttachments[0].loadOp = wgpu::LoadOp::Clear;
    gbufferAttachments[0].storeOp = wgpu::StoreOp::Store;
    gbufferAttachments[0].clearValue = {0, 0, 0, 0};

    gbufferAttachments[1].view = m_GBuffer.positionRoughness->createView();
    gbufferAttachments[1].loadOp = wgpu::LoadOp::Clear;
    gbufferAttachments[1].storeOp = wgpu::StoreOp::Store;
    gbufferAttachments[1].clearValue = {0, 0, 0, 0};

    gbufferAttachments[2].view = m_GBuffer.normalsAo->createView();
    gbufferAttachments[2].loadOp = wgpu::LoadOp::Clear;
    gbufferAttachments[2].storeOp = wgpu::StoreOp::Store;
    gbufferAttachments[2].clearValue = {0, 0, 0, 0};
 
    wgpu::RenderPassDepthStencilAttachment depthAttachment{};
    
    wgpu::TextureViewDescriptor depthViewDesc{};
    depthViewDesc.label = "Depth View",
    depthViewDesc.format = wgpu::TextureFormat::Depth32Float;
    depthViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    depthViewDesc.mipLevelCount = 1;
    depthViewDesc.arrayLayerCount = 1;

    depthAttachment.view = m_DepthTexture->getHandle().CreateView(&depthViewDesc);
    depthAttachment.depthClearValue = 1.f;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthAttachment.depthReadOnly = false;

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 3;
    renderPassDescription.colorAttachments = &gbufferAttachments[0];
    renderPassDescription.depthStencilAttachment = &depthAttachment;

    static uint64_t frameNr = 0;

    totalTriangles = 0;

    // static bool cacheTransforms = true;
    static std::set<uint32_t> uniqueIds;
    // wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();
    {   
        wgpu::RenderPassEncoder renderPass = encoder->BeginRenderPass(&renderPassDescription);
        renderPass.SetPipeline(m_RenderPipeline->getPipeline());
        renderPass.SetBindGroup(0, scene->getUniformsBindGroup()->get());
        renderPass.SetBindGroup(3, m_SamplerBindGroup->get());
        
        auto gameObjects = scene->getGameObjects();
        uniqueObjects = gameObjects.size();


        for(int i = 0; i < gameObjects.size(); ++i){
            GameObject* object = gameObjects.at(i);

            glm::mat4 transform = object->getTransform();

            const TriangleMesh* mesh = object->getMesh();
            const Material* material = object->getMaterial();

            if(mesh != nullptr && material != nullptr && mesh->isReady()){
                WGpuBindGroup* modelBindGroup = object->getModelBindGroup();
                if(m_CacheTransforms)
                    object->cacheTransform(transform, device);

                renderPass.SetBindGroup(1, modelBindGroup->get());
                    
                renderPass.SetBindGroup(2, material->getBindGroup()->get());

                renderPass.SetVertexBuffer(0, mesh->getVertexBuffer()->getHandle());

                WGpuIndexBuffer* indexBuffer = mesh->getIndexBuffer();
                renderPass.SetIndexBuffer(indexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(indexBuffer->getDataFormat()));
                renderPass.DrawIndexed(indexBuffer->getIndexCount());
                totalTriangles+= indexBuffer->getIndexCount() / 3;

                if(uniqueIds.count(mesh->getId()) <= 0) {
                    ++uniqueParts;
                    uniqueTriangles += indexBuffer->getIndexCount() / 3;
                }
                
                uniqueIds.insert(mesh->getId());
            }

            GameObject* child = object->getNext();
            while(child != nullptr) {
                transform = child->getTransform() * transform;
                
                const TriangleMesh* mesh = child->getMesh();
                const Material* material = child->getMaterial();

                if(mesh != nullptr && material != nullptr){ // && mesh->isReady()){
                    if(m_CacheTransforms) //TODO: need to move caching transform from renderer. In case mesh is not loaded first frame the transform will never be cached right now
                        child->cacheTransform(transform, device);

                    if(mesh->isReady()){ //Temporarily moved down since we don't have proper updating of transform buffer
                        WGpuBindGroup* modelBindGroup = child->getModelBindGroup();
                        renderPass.SetBindGroup(1, modelBindGroup->get());
                            
                        renderPass.SetBindGroup(2, material->getBindGroup()->get());

                        renderPass.SetVertexBuffer(0, mesh->getVertexBuffer()->getHandle());

                        WGpuIndexBuffer* indexBuffer = mesh->getIndexBuffer();
                        renderPass.SetIndexBuffer(indexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(indexBuffer->getDataFormat()));
                        renderPass.DrawIndexed(indexBuffer->getIndexCount());
                        totalTriangles += indexBuffer->getIndexCount() / 3;

                        if(uniqueIds.count(mesh->getId()) <= 0) {
                            ++uniqueParts;
                            uniqueTriangles += indexBuffer->getIndexCount() / 3;
                        }
                        
                        uniqueIds.insert(mesh->getId());
                    }
                    
                }

                child = child->getNext();
            }
        }
        
        renderPass.End();
    }
    frameNr++;

    // cacheTransforms = false;
    m_CacheTransforms = false;

    // Tímabundið
    EM_ASM({
        var elm = document.getElementById("TotalTriangleCount");
        elm.innerHTML = "# Total Triangles: " + $0;
        elm = document.getElementById("UniqueObjectCount");
        elm.innerHTML = "# Objects: " + $1;
        elm = document.getElementById("UniquePartCount");
        elm.innerHTML = "# Unique Parts: " + $2;
        elm = document.getElementById("UniqueTriangleCount");
        elm.innerHTML = "# Unique Triangles: " + $3;
    }, totalTriangles, uniqueObjects, uniqueParts, uniqueTriangles);

    // wgpu::TextureView backBuffer = swapChain->getCurrentFrameTexture();// swapChain.GetCurrentTextureView();

    // wgpu::RenderPassColorAttachment attachment{};
    // attachment.view = backBuffer;
    // attachment.loadOp = wgpu::LoadOp::Clear;
    // attachment.storeOp = wgpu::StoreOp::Store;
    // attachment.clearValue = {0, 0, 0, 1};

    // wgpu::RenderPassDescriptor lightingPassDescription{};
    // lightingPassDescription.colorAttachmentCount = 1;
    // lightingPassDescription.colorAttachments = &attachment;

    // {   
    //     wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&lightingPassDescription);
    //     renderPass.SetPipeline(m_LightingPipeline->getPipeline());
    //     renderPass.SetBindGroup(0, scene->getUniformsBindGroup()->get());
    //     renderPass.SetBindGroup(1, m_GBufferBindGroup->get());
    //     renderPass.SetBindGroup(2, scene->m_SamplerBindGroup->get());
    //     renderPass.Draw(3);
    //     renderPass.End();
    // }

    // wgpu::Queue queue = device->getHandle().GetQueue();
    // wgpu::CommandBuffer commands = encoder.Finish();
    // queue.Submit(1, &commands);

    // #ifdef WAIT_FOR_QUEUE
    // queue.OnSubmittedWorkDone(0, queueDoneCallback, nullptr);
    // #endif
    
}

void PBRRenderPipeline::light(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain)
{
    wgpu::Queue queue = device->getHandle().GetQueue();
    wgpu::TextureView backBuffer = swapChain->getCurrentFrameTexture();// swapChain.GetCurrentTextureView();

    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = backBuffer;// m_OutputTexture->createView();
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    attachment.clearValue = {0, 0, 0, 1};

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 1;
    renderPassDescription.colorAttachments = &attachment;

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();
        {   
            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(m_LightingPipeline->getPipeline());
            renderPass.SetBindGroup(0, scene->getUniformsBindGroup()->get());
            renderPass.SetBindGroup(1, m_GBufferBindGroup->get());
            renderPass.SetBindGroup(2, scene->m_SamplerBindGroup->get());
            renderPass.Draw(3);
            renderPass.End();
        }
        commands = encoder.Finish();
    }
    
    queue.Submit(1, &commands);
}