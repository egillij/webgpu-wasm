#include "PresentPipeline.h"

#include "PresentShader.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuPipeline.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuSampler.h"

PresentPipeline::PresentPipeline(WGpuTexture* texture, WGpuDevice* device)
: m_Pipeline(nullptr), m_Shader(nullptr), m_Texture(texture),
  m_TextureBindGroupLayout(nullptr), m_TextureBindGroup(nullptr), m_NearestSampler(nullptr)
{   
    m_TextureBindGroupLayout = new WGpuBindGroupLayout("Presentation Bind Group Layout");
    m_TextureBindGroupLayout->addTexture(TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBindGroupLayout->build(device);

    SamplerCreateInfo samplerInfo{};
    m_NearestSampler = new WGpuSampler("Sampler", &samplerInfo, device);

    m_TextureBindGroup = new WGpuBindGroup("Presentation Bind Group");
    m_TextureBindGroup->setLayout(m_TextureBindGroupLayout);
    m_TextureBindGroup->addTexture(m_Texture, TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBindGroup->addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBindGroup->build(device);

    ShaderDescription pbrDescription{};
    pbrDescription.shaderCode = presentCode;
    pbrDescription.colorTargets = {TextureFormat::BGRA8Unorm};
    m_Shader = new WGpuShader("Presentation_Shader", pbrDescription, device);
    m_Pipeline = new WGpuPipeline("Presentation Pipeline");
    m_Pipeline->addBindGroup(m_TextureBindGroupLayout);
    m_Pipeline->setShader(m_Shader);
    m_Pipeline->build(device, false);
}

PresentPipeline::~PresentPipeline()
{

}

void PresentPipeline::run(WGpuDevice* device, WGpuSwapChain* swapChain)
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

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();
        {   
            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(m_Pipeline->getPipeline());
            renderPass.SetBindGroup(0, m_TextureBindGroup->get());
            renderPass.Draw(3);
            renderPass.End();
        }
        commands = encoder.Finish();
    }
    
    queue.Submit(1, &commands);
}
