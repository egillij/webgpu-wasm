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
    WGpuIndexBuffer(WGpuDevice* device, const std::string& label, void* data, size_t size, IndexBufferFormat dataFormat);
    ~WGpuIndexBuffer() {};

    inline IndexBufferFormat getDataFormat() const {return m_DataFormat;}

private:
    IndexBufferFormat m_DataFormat;
};