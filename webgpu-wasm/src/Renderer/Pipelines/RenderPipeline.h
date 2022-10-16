#pragma once

#include <string>

class Scene;

class WGpuDevice;
class WGpuPipeline;
class WGpuSwapChain;
class WGpuTexture;

class RenderPipeline {
public:
    virtual void render(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain) = 0;

protected:
    RenderPipeline(const std::string& name);
    virtual ~RenderPipeline() = default ;

private:
    std::string m_Name;

protected:
    WGpuPipeline* m_Pipeline;

    WGpuTexture* m_DepthTexture;
};