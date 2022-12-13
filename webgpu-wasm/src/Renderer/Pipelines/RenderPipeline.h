#pragma once

#include <string>

class Scene;

class WGpuDevice;
class WGpuPipeline;
class WGpuSwapChain;
class WGpuTexture;

namespace wgpu {
    class Queue;
}

class RenderPipeline {
public:
    virtual ~RenderPipeline() = default ;
    virtual void run(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain, wgpu::Queue* queue) = 0;

    virtual WGpuTexture* getOutputTexture() = 0;
protected:
    RenderPipeline(const std::string& name);

private:
    std::string m_Name;
};