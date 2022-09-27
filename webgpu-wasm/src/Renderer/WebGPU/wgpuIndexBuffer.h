#pragma once

#include <webgpu/webgpu_cpp.h>

#include "wgpuBuffer.h"

enum class IndexBufferFormat : uint32_t {
    Undefined = static_cast<uint32_t>(wgpu::IndexFormat::Undefined),
    UNSIGNED_INT_32 = static_cast<uint32_t>(wgpu::IndexFormat::Uint32)
};

class WGpuIndexBuffer : public WGpuBuffer {
public:
    WGpuIndexBuffer() : m_DataFormat(IndexBufferFormat::Undefined) {};
    WGpuIndexBuffer(WGpuDevice* device, const std::string& label, void* data, uint64_t indexCount, IndexBufferFormat dataFormat);
    ~WGpuIndexBuffer() {};

    inline IndexBufferFormat getDataFormat() const {return m_DataFormat;}

    inline uint32_t getIndexCount() const {return m_IndexCount;}


private:
    IndexBufferFormat m_DataFormat;
    uint64_t m_IndexCount;
};