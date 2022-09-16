#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuDevice;
class WGpuBuffer;
class WGpuSampler;
class WGpuTexture;

enum class BufferBindingType : uint32_t {
    Undefined = static_cast<uint32_t>(wgpu::BufferBindingType::Undefined),
    Uniform = static_cast<uint32_t>(wgpu::BufferBindingType::Uniform),
    Storage = static_cast<uint32_t>(wgpu::BufferBindingType::Storage),
    ReadOnlyStorage = static_cast<uint32_t>(wgpu::BufferBindingType::ReadOnlyStorage),
};

enum class SamplerBindingType : uint32_t {
    Undefined = static_cast<uint32_t>(wgpu::SamplerBindingType::Undefined),
    Filtering = static_cast<uint32_t>(wgpu::SamplerBindingType::Filtering),
    NonFiltering = static_cast<uint32_t>(wgpu::SamplerBindingType::NonFiltering),
    Comparison = static_cast<uint32_t>(wgpu::SamplerBindingType::Comparison),
};

enum class TextureSampleType : uint32_t {
    Undefined = static_cast<uint32_t>(wgpu::TextureSampleType::Undefined),
    Float = static_cast<uint32_t>(wgpu::TextureSampleType::Float),
    UnfilterableFloat = static_cast<uint32_t>(wgpu::TextureSampleType::UnfilterableFloat),
    Depth = static_cast<uint32_t>(wgpu::TextureSampleType::Depth),
    Sint = static_cast<uint32_t>(wgpu::TextureSampleType::Sint),
    Uint = static_cast<uint32_t>(wgpu::TextureSampleType::Uint),
};

// TODO: only works for buffer binding layout. make it more general for other binding types (samplers, textures, ...)
class WGpuBindGroup {
public:
    WGpuBindGroup() : m_Label("") {};
    WGpuBindGroup(const std::string& label);
    ~WGpuBindGroup();

    void addBuffer(WGpuBuffer* buffer, BufferBindingType bindingType, uint64_t minBindingSize, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addSampler(WGpuSampler* sampler, SamplerBindingType bindingType, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addTexture(WGpuTexture* texture, TextureSampleType sampleType, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void build(WGpuDevice *device);
    const wgpu::BindGroup& get();
    wgpu::BindGroupLayout* getLayout();

private:
    struct Entry {
        uint32_t bindingSlot;
        wgpu::ShaderStage visibility;
        enum class Type {
            Buffer,
            Sampler,
            Texture,
            // StorageTexture
        } entryType;

        struct Buffer {
            WGpuBuffer* buffer;
            BufferBindingType type;
            uint64_t minBindingSize;
        } buffer;

        struct Sampler {
            WGpuSampler* sampler;
            SamplerBindingType type;
        } sampler;

        struct Texture {
            WGpuTexture* texture;
            TextureSampleType type;
        } texture;

    };

    std::string m_Label;

    wgpu::BindGroupLayout m_Layout;
    wgpu::BindGroup m_BindGroup;

    std::vector<Entry> m_Entries;

    
};

