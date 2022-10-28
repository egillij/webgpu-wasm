#pragma once

#include "Renderer/Pipelines/RenderPipeline.h"

class Scene;

class WGpuBindGroupLayout;
class WGpuBindGroup;
class WGpuDevice;
class WGpuShader;
class WGpuSampler;
class WGpuTexture;

class PresentPipeline final {
public:
    PresentPipeline(WGpuTexture* texture, WGpuDevice* device);
    ~PresentPipeline();

    void run(WGpuDevice* device, WGpuSwapChain* swapChain);

private:
    WGpuPipeline* m_Pipeline;

    WGpuShader* m_Shader;
    
    WGpuTexture* m_Texture;
    WGpuBindGroupLayout* m_TextureBindGroupLayout;
    WGpuBindGroup* m_TextureBindGroup;
    WGpuSampler* m_NearestSampler;
};