#include "CubemapVizualizationPipeline.h"

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

CubemapVizualizationPipeline::CubemapVizualizationPipeline(uint32_t width, uint32_t height, WGpuDevice* device)
: m_Pipeline(nullptr), m_Shader(nullptr), m_Cubemap(nullptr), m_DepthTexture(nullptr),
  m_TextureBindGroupLayout(nullptr), m_TextureBindGroup(nullptr), m_NearestSampler(nullptr)
{   
    m_TextureBindGroupLayout = new WGpuBindGroupLayout("Cubemap Vizualization Bind Group Layout");
    m_TextureBindGroupLayout->addCubemap(TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBindGroupLayout->build(device);

    TextureCreateInfo depthInfo{};
    depthInfo.format = TextureFormat::Depth32Float;
    depthInfo.height = height;
    depthInfo.width = width;
    depthInfo.usage = {TextureUsage::RenderAttachment};
    m_DepthTexture = new WGpuTexture("Depth texture", &depthInfo, device);

    SamplerCreateInfo samplerInfo{};
    m_NearestSampler = new WGpuSampler("Sampler", &samplerInfo, device);
 
    m_TextureBindGroup = new WGpuBindGroup("Cubemap Vizualization Texture Bind Group");
    // m_TextureBindGroup->setLayout(m_TextureBindGroupLayout);
    // m_TextureBindGroup->addCubemap(m_Cubemap, TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    // m_TextureBindGroup->addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    // m_TextureBindGroup->build(device);

    m_CameraUniformBindGroupLayout = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    m_CameraUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(CameraUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    m_CameraUniformBindGroupLayout->build(device);

    ShaderDescription pbrDescription{};
    pbrDescription.shaderCode = cubemapVizualizationCode;
    pbrDescription.colorTargets = {TextureFormat::BGRA8Unorm};
    pbrDescription.type = ShaderType::VERTEX_FRAGMENT;
    m_Shader = new WGpuShader("Cubemap_Vizualization_Shader", pbrDescription, device);

    m_Pipeline = new WGpuPipeline("Cubemap Vizualization Pipeline");
    m_Pipeline->addBindGroup(m_CameraUniformBindGroupLayout);
    m_Pipeline->addBindGroup(m_TextureBindGroupLayout);
    m_Pipeline->setShader(m_Shader);
    m_Pipeline->build(device, true);

    float aspect = static_cast<float>(height) / static_cast<float>(height);
    m_CameraUniforms.projection = glm::perspective(glm::radians(90.f), aspect, 0.1f, 100.f);
    glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);
    viewMatrix = glm::lookAt(glm::vec3(0.f), glm::normalize(direction), glm::vec3(0.f, 1.f, 0.f));
    m_CameraUniforms.viewRotation = glm::mat4(glm::mat3(viewMatrix));
    m_CameraUniformBuffer = new WGpuUniformBuffer(device, "Cubemap Viz Camera Uniformbuffer", sizeof(CameraUniforms));
    m_CameraUniformBindGroup = new WGpuBindGroup("Cubemap Viz Camera Uniform BG");
    m_CameraUniformBindGroup->addBuffer(m_CameraUniformBuffer, BufferBindingType::Uniform, sizeof(CameraUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    m_CameraUniformBindGroup->setLayout(m_CameraUniformBindGroupLayout);
    m_CameraUniformBindGroup->build(device);
}

CubemapVizualizationPipeline::~CubemapVizualizationPipeline()
{
    if(m_Pipeline) delete m_Pipeline;
    if(m_Shader) delete m_Shader;
    if(m_CameraUniformBindGroupLayout) delete m_CameraUniformBindGroupLayout;
    if(m_TextureBindGroup) delete m_TextureBindGroup;
    if(m_NearestSampler) delete m_NearestSampler;
    if(m_TextureBindGroupLayout) delete m_TextureBindGroupLayout;
}

void CubemapVizualizationPipeline::setCubemap(WGpuCubemap* cubemap, WGpuDevice* device)
{
    if(m_TextureBindGroup) delete m_TextureBindGroup;

    m_Cubemap = cubemap;
    m_TextureBindGroup = new WGpuBindGroup("Cubemap Vizualization Texture Bind Group");
    m_TextureBindGroup->setLayout(m_TextureBindGroupLayout);
    m_TextureBindGroup->addCubemap(m_Cubemap, TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBindGroup->addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBindGroup->build(device);
}

void CubemapVizualizationPipeline::run(WGpuDevice* device, WGpuSwapChain* swapChain)
{
    if(!swapChain) return;

    wgpu::Queue queue = device->getHandle().GetQueue();
    wgpu::TextureView backBuffer = swapChain->getCurrentFrameTexture();// swapChain.GetCurrentTextureView();

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

    depthAttachment.view = m_DepthTexture->getHandle().CreateView(&depthViewDesc);
    depthAttachment.depthClearValue = 1.f;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthAttachment.depthReadOnly = false;

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 1;
    renderPassDescription.colorAttachments = &attachment;
    renderPassDescription.depthStencilAttachment = &depthAttachment;

    static glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);
    static glm::mat4 rot = glm::rotate(glm::mat4(1.f), glm::radians(360.f/144.f/5.f), glm::vec3(0.f, 1.f, 0.f));
    direction = glm::vec3(rot*glm::vec4(direction, 0.f));
    viewMatrix = glm::lookAt(glm::vec3(0.f), glm::normalize(direction), glm::vec3(0.f, 1.f, 0.f));
    m_CameraUniforms.viewRotation = glm::mat4(glm::mat3(viewMatrix));
    //TODO: rotate view matrix to so we see the whole cubemap
    queue.WriteBuffer(m_CameraUniformBuffer->getHandle(), 0, (void*) &m_CameraUniforms, sizeof(CameraUniforms));

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
    
    queue.Submit(1, &commands);
}
