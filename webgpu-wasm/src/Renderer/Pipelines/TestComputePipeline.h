#pragma once

class WGpuBindGroupLayout;
class WGpuBindGroup;
class WGpuDevice;
class WGpuSwapChain;
class WGpuTexture;
class WGpuShader;

class ComputePipeline;

#include "Renderer/Pipelines/PresentPipeline.h"

#include <webgpu/webgpu_cpp.h>

class TestComputePipeline {
public:
    TestComputePipeline(WGpuDevice* device);
    ~TestComputePipeline();
    
    void run(WGpuDevice* device, WGpuSwapChain* swapChain);
    void present();
private:
    WGpuDevice* m_Device;
    WGpuSwapChain* m_SwapChain;

    WGpuTexture* m_Texture;

    ComputePipeline* m_Pipeline;
    WGpuShader* m_Shader;

    WGpuBindGroupLayout* m_BindGroupLayout;
    WGpuBindGroup* m_BindGroup;

    PresentPipeline* m_PresentPipeline;
};