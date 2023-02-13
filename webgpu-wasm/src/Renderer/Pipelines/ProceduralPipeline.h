// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <string>

class Scene;

class WGpuDevice;
class WGpuPipeline;
class WGpuSwapChain;
class WGpuBindGroup;
class WGpuBindGroupLayout;

namespace wgpu {
    class Queue;
}

class ProceduralPipeline {
public:
    virtual void run(WGpuBindGroup* textureBindGroup, WGpuDevice* device, wgpu::Queue* queue) = 0;

    virtual WGpuBindGroupLayout* getBindGroupLayout() = 0;

protected:
    ProceduralPipeline(const std::string& name) : m_Name(name) {};
    virtual ~ProceduralPipeline() = default ;

private:
    std::string m_Name;
};