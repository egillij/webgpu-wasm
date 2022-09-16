#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>

class WGpuDevice;

enum class AddressMode : uint32_t {
    Repeat = static_cast<uint32_t>(wgpu::AddressMode::Repeat),
    MirrorRepeat = static_cast<uint32_t>(wgpu::AddressMode::MirrorRepeat),
    ClampToEdge = static_cast<uint32_t>(wgpu::AddressMode::ClampToEdge),
};

enum class FilterMode {
    Nearest = static_cast<uint32_t>(wgpu::FilterMode::Nearest),
    Linear = static_cast<uint32_t>(wgpu::FilterMode::Linear)
};

enum class CompareFunction {
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

struct SamplerCreateInfo {
    AddressMode addressModeU = AddressMode::ClampToEdge;
    AddressMode addressModeV = AddressMode::ClampToEdge;
    AddressMode addressModeW = AddressMode::ClampToEdge;
    FilterMode magFilter = FilterMode::Nearest;
    FilterMode minFilter = FilterMode::Nearest;
    FilterMode mipmapFilter = FilterMode::Nearest;
    float lodMinClamp = 0.0f;
    float lodMaxClamp = 1000.0f;
    CompareFunction compare = CompareFunction::Undefined;
    uint16_t maxAnisotropy = 1;
};

class WGpuSampler {
public:
    WGpuSampler(const std::string& label, const SamplerCreateInfo* createInfo, WGpuDevice* device);
    ~WGpuSampler();

    inline wgpu::Sampler& getHandle() { return m_Sampler; }

private:
    wgpu::Sampler m_Sampler;
};