#pragma once

class WGpuDevice;
class WGpuSwapChain;
class WGpuTexture;

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

    //TODO: nota eigin klasa
    ComputePipeline* m_Pipeline;

    wgpu::ShaderModule m_Module;
    wgpu::BindGroup m_BindGroup;

    PresentPipeline* m_PresentPipeline;
};