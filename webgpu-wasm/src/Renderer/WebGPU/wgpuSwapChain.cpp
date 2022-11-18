#include "wgpuSwapChain.h"

#include "wgpuDevice.h"

WGpuSwapChain::WGpuSwapChain(WGpuDevice* device, uint32_t width, uint32_t height)
: m_Device(device)
{
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
    canvasDesc.selector = "#canvas";

    wgpu::SurfaceDescriptor surfDescription{};
    surfDescription.nextInChain = &canvasDesc;
    wgpu::Instance instance{};
    m_Surface = instance.CreateSurface(&surfDescription);
    createSwapChain(width, height);
}

wgpu::TextureView WGpuSwapChain::getCurrentFrameTexture() const
{
    return m_SwapChain.GetCurrentTextureView();
}

// void WGpuSwapChain::present()
// {
//     m_SwapChain.Present();
// }

void WGpuSwapChain::createSwapChain(uint32_t width, uint32_t height)
{
    wgpu::SwapChainDescriptor description{};
    description.usage = wgpu::TextureUsage::RenderAttachment;
    description.format = wgpu::TextureFormat::BGRA8Unorm;
    description.width = width;
    description.height = height;
    description.presentMode = wgpu::PresentMode::Fifo;
    m_SwapChain = m_Device->getHandle().CreateSwapChain(m_Surface, &description);
}

