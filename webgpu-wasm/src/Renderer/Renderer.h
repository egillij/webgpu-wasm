#pragma once

#include <cstdint>

class WGpuDevice;
class WGpuSwapChain;
class WGpuPipeline;
class WGpuTexture;
class WGpuSampler;
class WGpuBindGroup;
class WGpuBindGroupLayout;

class WGpuShader;

class Scene;

class RenderPipeline;
class PresentPipeline;

class Renderer {
public:
    Renderer(uint32_t width, uint32_t height, WGpuDevice* device);
    ~Renderer();

    void render(Scene* scene);

    void present();

private:
    WGpuDevice* m_Device = nullptr;
    WGpuSwapChain* m_SwapChain = nullptr;

    RenderPipeline* m_Pipeline;
    
    PresentPipeline* m_PresentPipeline;
};