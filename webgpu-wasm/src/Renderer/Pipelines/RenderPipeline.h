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

typedef void(*PipelineDoneCallback)(void* args);

class RenderPipeline {
public:
    virtual ~RenderPipeline() = default ;
    virtual void run(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain, wgpu::Queue* queue, PipelineDoneCallback callback, void* args) = 0;

    virtual WGpuTexture* getOutputTexture() = 0;
    virtual WGpuTexture* getDepthTexture() = 0;
protected:
    RenderPipeline(const std::string& name);

private:
    std::string m_Name;
};