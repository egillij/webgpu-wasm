#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuBindGroupLayout;
class WGpuShader;
class WGpuDevice;

enum class CullMode : uint32_t {
    None = static_cast<uint32_t>(wgpu::CullMode::None),
    Back = static_cast<uint32_t>(wgpu::CullMode::Back),
    Front = static_cast<uint32_t>(wgpu::CullMode::Front)
};

class WGpuPipeline {
public:
    WGpuPipeline(const std::string& name);
    ~WGpuPipeline();

    void addBindGroup(WGpuBindGroupLayout* bindgroup);
    
    //void setVertexBufferLayout(....);
    void setShader(WGpuShader* shader);

    void setCullMode(CullMode cullmode);

    void build(WGpuDevice* device, bool forRendering);

    const wgpu::RenderPipeline& getPipeline() const { return m_Pipeline; }

private:
    std::string m_Name;
    std::vector<WGpuBindGroupLayout*> m_BindGroups;
    WGpuShader* m_Shader;

    CullMode m_CullMode;

    wgpu::RenderPipeline m_Pipeline;
};