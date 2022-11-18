#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuBindGroupLayout;
class WGpuShader;
class WGpuDevice;

class WGpuPipeline {
public:
    WGpuPipeline(const std::string& name);
    ~WGpuPipeline();

    void addBindGroup(WGpuBindGroupLayout* bindgroup);
    
    //void setVertexBufferLayout(....);
    void setShader(WGpuShader* shader);

    void build(WGpuDevice* device, bool forRendering);

    const wgpu::RenderPipeline& getPipeline() const { return m_Pipeline; }

private:
    std::string m_Name;
    std::vector<WGpuBindGroupLayout*> m_BindGroups;
    WGpuShader* m_Shader;

    wgpu::RenderPipeline m_Pipeline;
};