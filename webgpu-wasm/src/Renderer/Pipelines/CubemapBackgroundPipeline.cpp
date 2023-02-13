// Copyright 2023 Egill Ingi Jacobsen

#include "CubemapBackgroundPipeline.h"

#include "CubemapShaders.h"
#include "Application.h"

#include "Renderer/Geometry/TriangleMesh.h"
#include "Renderer/Geometry/GeometrySystem.h"


#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuPipeline.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"

#include <glm/gtc/matrix_transform.hpp>

CubemapBackgroundPipeline::CubemapBackgroundPipeline(WGpuDevice* device)
: m_Pipeline(nullptr), m_Shader(nullptr), m_Cubemap(nullptr), m_DepthTexture(nullptr),
  m_TextureBindGroupLayout(nullptr), m_TextureBindGroup(nullptr), m_NearestSampler(nullptr)
{   
    m_TextureBindGroupLayout = new WGpuBindGroupLayout("Cubemap Background BGL");
    m_TextureBindGroupLayout->addCubemap(TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBindGroupLayout->build(device);

    SamplerCreateInfo samplerInfo{};
    m_NearestSampler = new WGpuSampler("Sampler", &samplerInfo, device);
 
    // m_TextureBindGroup = new WGpuBindGroup("Cubemap Vizualization Texture Bind Group");
    // m_TextureBindGroup->setLayout(m_TextureBindGroupLayout);
    // m_TextureBindGroup->addCubemap(m_Cubemap, TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    // m_TextureBindGroup->addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    // m_TextureBindGroup->build(device);

    m_CameraUniformBindGroupLayout = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    m_CameraUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(CameraUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    m_CameraUniformBindGroupLayout->build(device);

    ShaderDescription pbrDescription{};
    pbrDescription.shaderCode = cubemapVizualizationCode;
    pbrDescription.colorTargets = {TextureFormat::RGBA32Float};
    pbrDescription.type = ShaderType::VERTEX_FRAGMENT;
    m_Shader = new WGpuShader("Cubemap Background Shader", pbrDescription, device);

    m_Pipeline = new WGpuPipeline("Cubemap Background Pipeline");
    m_Pipeline->addBindGroup(m_CameraUniformBindGroupLayout);
    m_Pipeline->addBindGroup(m_TextureBindGroupLayout);
    m_Pipeline->setShader(m_Shader);
    m_Pipeline->setDepth(DepthFormat::Depth32Float, DepthCompare::LessEqual);
    m_Pipeline->build(device, true);

    m_CameraUniforms.projection = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 100.f);
    
    // m_CameraUniforms.viewRotation = glm::mat4(glm::mat3(viewMatrix));
    m_CameraUniformBuffer = new WGpuUniformBuffer(device, "Cubemap Background Camera Uniformbuffer", sizeof(CameraUniforms));
    m_CameraUniformBindGroup = new WGpuBindGroup("Cubemap Background Camera Uniform BG");
    m_CameraUniformBindGroup->addBuffer(m_CameraUniformBuffer, BufferBindingType::Uniform, sizeof(CameraUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    m_CameraUniformBindGroup->setLayout(m_CameraUniformBindGroupLayout);
    m_CameraUniformBindGroup->build(device);
}

CubemapBackgroundPipeline::~CubemapBackgroundPipeline()
{
    if(m_Pipeline) delete m_Pipeline;
    if(m_Shader) delete m_Shader;
    if(m_CameraUniformBindGroupLayout) delete m_CameraUniformBindGroupLayout;
    if(m_TextureBindGroup) delete m_TextureBindGroup;
    if(m_NearestSampler) delete m_NearestSampler;
    if(m_TextureBindGroupLayout) delete m_TextureBindGroupLayout;
}

void CubemapBackgroundPipeline::setCubemap(WGpuTexture* depthTexture, WGpuCubemap* cubemap, WGpuDevice* device)
{
    if(m_TextureBindGroup) delete m_TextureBindGroup;

    m_Cubemap = cubemap;
    m_TextureBindGroup = new WGpuBindGroup("Cubemap Background Texture Bind Group");
    m_TextureBindGroup->setLayout(m_TextureBindGroupLayout);
    m_TextureBindGroup->addCubemap(m_Cubemap, TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBindGroup->addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBindGroup->build(device);

    m_DepthTexture = depthTexture;
}

void CubemapBackgroundPipeline::run(const glm::mat4& viewMatrix, WGpuDevice* device, WGpuTexture* target, wgpu::Queue* queue)
{
    // wgpu::Queue queue = device->getHandle().GetQueue();

    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = target->createView();
    attachment.loadOp = wgpu::LoadOp::Load;
    attachment.storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassDepthStencilAttachment depthAttachment{};

    wgpu::TextureViewDescriptor depthViewDesc{};
    depthViewDesc.label = "Depth View",
    depthViewDesc.format = wgpu::TextureFormat::Depth32Float;
    depthViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    depthViewDesc.mipLevelCount = 1;
    depthViewDesc.arrayLayerCount = 1;

    depthAttachment.view = m_DepthTexture->getHandle().CreateView(&depthViewDesc);
    depthAttachment.depthClearValue = 1.f;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Load;
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthAttachment.depthReadOnly = false;

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 1;
    renderPassDescription.colorAttachments = &attachment;
    renderPassDescription.depthStencilAttachment = &depthAttachment;

    m_CameraUniforms.viewRotation = glm::mat4(glm::mat3(viewMatrix));
    //TODO: rotate view matrix to so we see the whole cubemap
    queue->WriteBuffer(m_CameraUniformBuffer->getHandle(), 0, (void*) &m_CameraUniforms, sizeof(CameraUniforms));

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();
        {   
            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(m_Pipeline->getPipeline());
            renderPass.SetBindGroup(0, m_CameraUniformBindGroup->get());
            renderPass.SetBindGroup(1, m_TextureBindGroup->get());

            //TODO: get cube geometry and draw it. Make a function for it in the GeometrySystem?
            static GeometrySystem* geos = Application::get()->getGeometrySystem();
            static const TriangleMesh* cube = geos->getCube();
            renderPass.SetVertexBuffer(0, cube->getVertexBuffer()->getHandle());
            static WGpuIndexBuffer* cubeIndexBuffer = cube->getIndexBuffer();
            renderPass.SetIndexBuffer(cubeIndexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(cubeIndexBuffer->getDataFormat()));
            renderPass.DrawIndexed(cubeIndexBuffer->getIndexCount());
            renderPass.End();
        }
        commands = encoder.Finish();
    }
    
    queue->Submit(1, &commands);
}
