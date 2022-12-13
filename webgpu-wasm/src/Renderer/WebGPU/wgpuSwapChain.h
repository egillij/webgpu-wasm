#pragma once

#include <webgpu/webgpu_cpp.h>

class WGpuDevice;

class WGpuSwapChain final {
public:
    WGpuSwapChain() {};
    WGpuSwapChain(WGpuDevice* device, uint32_t width, uint32_t height);
    ~WGpuSwapChain();

    wgpu::TextureView getCurrentFrameTexture() const;

    // void present();

private:
    void createSwapChain(uint32_t width, uint32_t height);

private:
    WGpuDevice* m_Device;
    wgpu::SwapChain m_SwapChain;
    wgpu::Surface m_Surface;
};