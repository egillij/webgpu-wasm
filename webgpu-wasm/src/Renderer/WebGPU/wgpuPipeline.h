// Copyright 2023 Egill Ingi Jacobsen

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

enum class DepthCompare : uint32_t {
    Undefined = static_cast<uint32_t>(wgpu::CompareFunction::Undefined),
    Never = static_cast<uint32_t>(wgpu::CompareFunction::Never),
    Less = static_cast<uint32_t>(wgpu::CompareFunction::Less),
    LessEqual = static_cast<uint32_t>(wgpu::CompareFunction::LessEqual),
    Greater = static_cast<uint32_t>(wgpu::CompareFunction::Greater),
    GreaterEqual = static_cast<uint32_t>(wgpu::CompareFunction::GreaterEqual),
    Equal = static_cast<uint32_t>(wgpu::CompareFunction::Equal),
    NotEqual = static_cast<uint32_t>(wgpu::CompareFunction::NotEqual),
    Always = static_cast<uint32_t>(wgpu::CompareFunction::Always),
};

enum class DepthFormat : uint32_t {
    Depth16Unorm = static_cast<uint32_t>(wgpu::TextureFormat::Depth16Unorm),
    Depth24Plus = static_cast<uint32_t>(wgpu::TextureFormat::Depth24Plus),
    Depth24PlusStencil8 = static_cast<uint32_t>(wgpu::TextureFormat::Depth24PlusStencil8),
    Depth32Float = static_cast<uint32_t>(wgpu::TextureFormat::Depth32Float),
    Depth32FloatStencil8 = static_cast<uint32_t>(wgpu::TextureFormat::Depth32FloatStencil8),
};

class WGpuPipeline {
public:
    WGpuPipeline(const std::string& name);
    ~WGpuPipeline();

    //TODO: use a PipelineDescriptor struct instead of all these functions?
    void addBindGroup(WGpuBindGroupLayout* bindgroup);
    
    //void setVertexBufferLayout(....);
    void setShader(WGpuShader* shader);

    void setCullMode(CullMode cullmode);

    void setDepth(DepthFormat format, DepthCompare comparFunction);

    void build(WGpuDevice* device, bool forRendering);

    const wgpu::RenderPipeline& getPipeline() const { return m_Pipeline; }

private:
    std::string m_Name;
    std::vector<WGpuBindGroupLayout*> m_BindGroups;
    WGpuShader* m_Shader;

    CullMode m_CullMode;

    struct DepthInfo {
        bool active = false;
        DepthFormat format = DepthFormat::Depth24Plus;
        DepthCompare compareFunction = DepthCompare::LessEqual;
    } m_DepthInfo;

    wgpu::RenderPipeline m_Pipeline;
};