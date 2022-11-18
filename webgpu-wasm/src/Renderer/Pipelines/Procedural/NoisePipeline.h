#pragma once

#include "Renderer/Pipelines/ProceduralPipeline.h"

class ComputePipeline;
class WGpuShader;
class WGpuBindGroupLayout;
class WGpuBindGroup;

class NoisePipeline : public ProceduralPipeline {
public:
    NoisePipeline(WGpuDevice* device);
    ~NoisePipeline();

    virtual void run(WGpuBindGroup* textureBindGroup, WGpuDevice* device, wgpu::Queue* queue) override;

    virtual WGpuBindGroupLayout* getBindGroupLayout() override { return m_BindGroupLayout; }

private:
    ComputePipeline* m_Pipeline;
    WGpuShader* m_Shader;

    WGpuBindGroupLayout* m_BindGroupLayout;
};