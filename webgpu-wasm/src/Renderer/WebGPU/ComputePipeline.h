#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuDevice;
class WGpuBindGroupLayout;
class WGpuShader;

class ComputePipeline {
public:
    ComputePipeline(const std::string& label);
    ~ComputePipeline();

    void addBindGroup(WGpuBindGroupLayout* bindgroup);

    void setShader(WGpuShader* shader);

    void build(WGpuDevice* device);

    const wgpu::ComputePipeline& getPipeline() const { return m_Pipeline; }

private:
    std::string m_Label;

    std::vector<WGpuBindGroupLayout*> m_BindGroups;
    WGpuShader* m_Shader;

    wgpu::ComputePipeline m_Pipeline;

};