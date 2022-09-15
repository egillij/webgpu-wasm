#pragma once

#include "wgpuDevice.h"
#include "wgpuBuffer.h"

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

// class WGpuDevice;
// class WGpuBuffer;

enum class BufferBindingType : uint32_t {
    Undefined = static_cast<uint32_t>(wgpu::BufferBindingType::Undefined),
    Uniform = static_cast<uint32_t>(wgpu::BufferBindingType::Uniform),
    Storage = static_cast<uint32_t>(wgpu::BufferBindingType::Storage),
    ReadOnlyStorage = static_cast<uint32_t>(wgpu::BufferBindingType::ReadOnlyStorage)
};


// TODO: only works for buffer binding layout. make it more general for other binding types (samplers, textures, ...)
class WGpuBindGroup {
public:
    WGpuBindGroup() : m_Label("") {};
    WGpuBindGroup(const std::string& label);
    ~WGpuBindGroup();

    void addEntry(WGpuBuffer* buffer, BufferBindingType bindingType, uint64_t minBindingSize, uint32_t bindingSlot, wgpu::ShaderStage shaderStage);

    void build(WGpuDevice *device);
    const wgpu::BindGroup& get();
    wgpu::BindGroupLayout* getLayout();

private:
    struct Entry {
        uint32_t bindingSlot;
        wgpu::ShaderStage visibility;
        struct Buffer {
            WGpuBuffer* buffer;
            BufferBindingType type;
            uint64_t minBindingSize;
        } buffer;
    };

    std::string m_Label;

    wgpu::BindGroupLayout m_Layout;
    wgpu::BindGroup m_BindGroup;

    std::vector<Entry> m_Entries;

    
};

