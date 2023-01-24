#pragma once

#include <cstdint>

#include "Renderer/Pipelines/TestComputePipeline.h"

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
class CubemapVizualizationPipeline;
class CubemapGenerationPipeline;
class CubemapBackgroundPipeline;


class Renderer {
public:
    Renderer(uint32_t width, uint32_t height, WGpuDevice* device);
    ~Renderer();

    void render(Scene* scene);
    void renderBackground();
    void present();

private:
    WGpuDevice* m_Device = nullptr;
    WGpuSwapChain* m_SwapChain = nullptr;

    RenderPipeline* m_Pipeline;

    CubemapBackgroundPipeline* m_Background;
    
    PresentPipeline* m_PresentPipeline;

    TestComputePipeline* m_Compute;

    CubemapVizualizationPipeline* m_CubemapVizPipeline;

    CubemapGenerationPipeline* m_DiffuseConvolutionPipeline;

    Scene* m_Scene;
};