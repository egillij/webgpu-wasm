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

class Renderer {
public:
    Renderer(uint32_t width, uint32_t height, WGpuDevice* device);
    ~Renderer();

    void render(Scene* scene);

private:
    WGpuDevice* m_Device = nullptr;
    WGpuSwapChain* m_SwapChain = nullptr;
    WGpuPipeline* m_Pipeline = nullptr;
    WGpuTexture* m_DepthTexture = nullptr;

    WGpuBindGroupLayout* m_ModelUniformBindGroupLayout = nullptr;
    WGpuBindGroupLayout* m_MaterialBindGroupLayout = nullptr;
    WGpuBindGroupLayout* m_SceneUniformBindGroupLayout = nullptr;

    //Temp. Move shader to material class
    WGpuShader* shader = nullptr;

    WGpuBindGroupLayout* materialBindGroupLayout = nullptr;
    WGpuBindGroup* materialBindGroup = nullptr;
    WGpuTexture* texture = nullptr;
    WGpuSampler* sampler = nullptr;

};