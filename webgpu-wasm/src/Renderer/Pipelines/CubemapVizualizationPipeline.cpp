#include "CubemapVizualizationPipeline.h"

#include "CubemapShaders.h"


#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuPipeline.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"

#include <glm/gtc/matrix_transform.hpp>

CubemapVizualizationPipeline::CubemapVizualizationPipeline(uint32_t width, uint32_t height, WGpuDevice* device)
: m_Pipeline(nullptr), m_Shader(nullptr), m_Cubemap(nullptr),
  m_TextureBindGroupLayout(nullptr), m_TextureBindGroup(nullptr), m_NearestSampler(nullptr)
{   
    m_TextureBindGroupLayout = new WGpuBindGroupLayout("Cubemap Vizualization Bind Group Layout");
    m_TextureBindGroupLayout->addTexture(TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBindGroupLayout->build(device);

    SamplerCreateInfo samplerInfo{};
    m_NearestSampler = new WGpuSampler("Sampler", &samplerInfo, device);
 
    m_TextureBindGroup = new WGpuBindGroup("Cubemap Vizualization Texture Bind Group");
    m_TextureBindGroup->setLayout(m_TextureBindGroupLayout);
    m_TextureBindGroup->addCubemap(m_Cubemap, TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBindGroup->addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBindGroup->build(device);

    m_CameraUniformBindGroupLayout = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    m_CameraUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(CameraUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    m_CameraUniformBindGroupLayout->build(device);

    ShaderDescription pbrDescription{};
    pbrDescription.shaderCode = cubemapVizualizationCode;
    pbrDescription.colorTargets = {TextureFormat::BGRA8Unorm};
    m_Shader = new WGpuShader("Cubemap_Vizualization_Shader", pbrDescription, device);

    m_Pipeline = new WGpuPipeline("Cubemap Vizualization Pipeline");
    m_Pipeline->addBindGroup(m_CameraUniformBindGroupLayout);
    m_Pipeline->addBindGroup(m_TextureBindGroupLayout);
    m_Pipeline->setShader(m_Shader);
    m_Pipeline->build(device, false);

    float aspect = static_cast<float>(height) / static_cast<float>(height);
    m_CameraUniforms.projection = glm::perspective(glm::radians(90.f), aspect, 0.1f, 100.f);
    viewMatrix = glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
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

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 1;
    renderPassDescription.colorAttachments = &attachment;

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
            renderPass.Draw(3);
            renderPass.End();
        }
        commands = encoder.Finish();
    }
    
    queue.Submit(1, &commands);
}
