#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuDevice;
class WGpuBuffer;
class WGpuSampler;
class WGpuTexture;
class WGpuCubemap;

enum class TextureFormat : uint32_t;

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

class WGpuBindGroupLayout {
public:
    WGpuBindGroupLayout() : m_Label("") {};
    WGpuBindGroupLayout(const std::string& label);
    ~WGpuBindGroupLayout();

    //TODO: do some validation against the layout when adding entries
    void addBuffer(BufferBindingType bindingType, uint64_t minBindingSize, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addSampler(SamplerBindingType bindingType, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addTexture(TextureSampleType sampleType, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addStorageTexture(wgpu::StorageTextureAccess access, TextureFormat format, wgpu::TextureViewDimension dim, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addCubemap(TextureSampleType sampleType, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void build(WGpuDevice *device);
    wgpu::BindGroupLayout* get();

private:
    struct LayoutEntry {
        uint32_t bindingSlot;
        wgpu::ShaderStage visibility;
        enum class Type {
            Buffer,
            Sampler,
            Texture,
            StorageTexture,
            Cubemap
        } entryType;

        struct Buffer {
            BufferBindingType type;
            uint64_t minBindingSize;
        } buffer;

        struct Sampler {
            SamplerBindingType type;
        } sampler;

        struct Texture {
            TextureSampleType type;
        } texture;

        struct StorageTexture {
            wgpu::StorageTextureAccess access;
            TextureFormat format;
            wgpu::TextureViewDimension dim;
        } storagTexture;

        struct Cubemap {
            TextureSampleType type;
        } cubemap;
    };

    std::string m_Label;

    wgpu::BindGroupLayout m_Layout;

    std::vector<LayoutEntry> m_Entries;
};

// TODO: only works for buffer binding layout. make it more general for other binding types (samplers, textures, ...)
class WGpuBindGroup {
public:
    WGpuBindGroup() : m_Label("") {};
    WGpuBindGroup(const std::string& label);
    ~WGpuBindGroup();

    void setLayout(WGpuBindGroupLayout* layout);

    void addBuffer(WGpuBuffer* buffer, BufferBindingType bindingType, uint64_t minBindingSize, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addSampler(WGpuSampler* sampler, SamplerBindingType bindingType, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addTexture(WGpuTexture* texture, TextureSampleType sampleType, uint32_t bindingSlot, wgpu::ShaderStage visibility);
    void addCubemap(WGpuCubemap* cubemap, TextureSampleType sampleType, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void addStorageTexture(WGpuTexture* texture, uint32_t bindingSlot, wgpu::ShaderStage visibility);

    void build(WGpuDevice *device);

    const wgpu::BindGroup& get();
    WGpuBindGroupLayout* getLayout();

private:
    struct Entry {
        uint32_t bindingSlot;
        wgpu::ShaderStage visibility;
        enum class Type {
            Buffer,
            Sampler,
            Texture,
            StorageTexture,
            Cubemap
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

        struct StorageTexture {
            WGpuTexture* texture;
        } storageTexture;

        struct Cubemap {
            WGpuCubemap* cubemap;
            TextureSampleType type;
        } cubemap;

    };

    std::string m_Label;

    WGpuBindGroupLayout* m_Layout;
    wgpu::BindGroup m_BindGroup;

    std::vector<Entry> m_Entries;

    
};

