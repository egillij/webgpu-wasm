#pragma once

#include "Renderer/Pipelines/RenderPipeline.h"

class Scene;

class WGpuBindGroupLayout;
class WGpuDevice;
class WGpuShader;

class PBRRenderPipeline final : public RenderPipeline {
public:
    PBRRenderPipeline(uint32_t width, uint32_t height, WGpuDevice* device);
    ~PBRRenderPipeline();

    virtual void render(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain) override;

private:
    WGpuBindGroupLayout* m_ModelUniformBindGroupLayout;
    WGpuBindGroupLayout* m_MaterialBindGroupLayout;
    WGpuBindGroupLayout* m_SceneUniformBindGroupLayout;
    WGpuBindGroupLayout* m_SamplerBindGroupLayout;

    WGpuShader* m_Shader;
};