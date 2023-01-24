#pragma once

#include "Renderer/Pipelines/RenderPipeline.h"

class Scene;

class WGpuBindGroupLayout;
class WGpuBindGroup;
class WGpuDevice;
class WGpuShader;
class WGpuSampler;

namespace wgpu {
    class CommandBuffer;
    class CommandEncoder;
};

struct GBuffer{
    WGpuTexture* albedoMetallic = nullptr;
    WGpuTexture* positionRoughness = nullptr;
    WGpuTexture* normalsAo = nullptr;
    // WGpuTexture* 
};

class PBRRenderPipeline final : public RenderPipeline {
public:
    PBRRenderPipeline(uint32_t width, uint32_t height, WGpuDevice* device);
    ~PBRRenderPipeline();
 
    virtual void run(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain, wgpu::Queue* queue, PipelineDoneCallback callback, void* args) override;
    virtual WGpuTexture* getOutputTexture() override { return m_OutputTexture; };

    virtual WGpuTexture* getDepthTexture() override { return m_DepthTexture; }
    
// private:
    void render(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain, wgpu::CommandEncoder* commandEncoder);//, wgpu::CommandEncoder& encoder);
    void light(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain);//, wgpu::CommandEncoder* commandEncoder);//, wgpu::CommandEncoder& encoder);

private:
    GBuffer m_GBuffer;

    WGpuBindGroupLayout* m_ModelUniformBindGroupLayout;
    WGpuBindGroupLayout* m_MaterialBindGroupLayout;
    WGpuBindGroupLayout* m_SceneUniformBindGroupLayout;
    WGpuBindGroupLayout* m_SamplerBindGroupLayout;

    WGpuShader* m_RenderShader;
    WGpuShader* m_LightingShader;

    WGpuPipeline* m_RenderPipeline;

    WGpuPipeline* m_LightingPipeline;
    WGpuBindGroupLayout* m_GBufferBindGroupLayout;
    WGpuBindGroup* m_GBufferBindGroup;

    WGpuBindGroup* m_SamplerBindGroup;
    WGpuBindGroup* m_SamplerBindGroup2;
    WGpuSampler* m_NearestSampler;
    WGpuSampler* m_NearestSampler2;

    WGpuTexture* m_DepthTexture;
    WGpuTexture* m_OutputTexture;

    bool m_CacheTransforms;

    struct TaskComplete {
        PipelineDoneCallback  callback;
        void* args;
    } m_CompletionTask;
};