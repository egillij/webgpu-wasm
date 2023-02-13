// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>

class WGpuDevice;

enum class BufferUsage : uint32_t {
    None = static_cast<uint32_t>(wgpu::BufferUsage::None),
    Vertex = static_cast<uint32_t>(wgpu::BufferUsage::Vertex),
    Index = static_cast<uint32_t>(wgpu::BufferUsage::Index),
    Uniform = static_cast<uint32_t>(wgpu::BufferUsage::Uniform),
    CopyDst = static_cast<uint32_t>(wgpu::BufferUsage::CopyDst)
};

inline BufferUsage operator|(BufferUsage a, BufferUsage b){
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

class WGpuBuffer {
public:
    struct BufferDescription {
        std::string label;
        BufferUsage bufferUsage;
        uint64_t size;
        void* data;
    };

public:
    WGpuBuffer() : m_Device(nullptr) {};
    WGpuBuffer(WGpuDevice* device);
    WGpuBuffer(WGpuDevice* device, const BufferDescription& description);
    ~WGpuBuffer();

    void create(const BufferDescription& description);

    wgpu::Buffer& getHandle() { return m_Buffer; }
    inline uint64_t getSize() const { return m_Size; };

private:
    WGpuDevice* m_Device;
    wgpu::Buffer m_Buffer;
    uint64_t m_Size;
};