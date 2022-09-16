#pragma once

#include <webgpu/webgpu_cpp.h>

#include <vector>

class WGpuBindGroup;
class WGpuShader;
class WGpuDevice;

class WGpuPipeline {
public:
    WGpuPipeline();
    ~WGpuPipeline();

    void addBindGroup(WGpuBindGroup* bindgroup);
    
    //void setVertexBufferLayout(....);
    void setShader(WGpuShader* shader);

    void build(WGpuDevice* device);

    const wgpu::RenderPipeline& getPipeline() const { return m_Pipeline; }

private:
    std::vector<WGpuBindGroup*> m_BindGroups;
    WGpuShader* m_Shader;

    wgpu::RenderPipeline m_Pipeline;
};