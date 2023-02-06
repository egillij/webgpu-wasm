#pragma once

class WGpuDevice;
class WGpuSwapChain;
class WGpuTexture;
class WGpuShader;
class ComputePipeline;
class WGpuBindGroupLayout;
class WGpuBindGroup;

namespace wgpu {
    class Queue;
};

class PathTracingPipeline {
public:
    PathTracingPipeline();
    PathTracingPipeline(WGpuDevice* device);
    ~PathTracingPipeline();

    void run(WGpuDevice* device, wgpu::Queue* queue);

    WGpuTexture* getTargetBuffer();

private:
    bool m_IsValid = false;
    WGpuTexture* m_TargetTexture = nullptr;

    WGpuShader* m_PathTraceShader = nullptr;

    WGpuBindGroupLayout* m_BindGroupLayout = nullptr;
    WGpuBindGroup* m_BindGroup = nullptr;

    ComputePipeline* m_Pipeline = nullptr;
    
};