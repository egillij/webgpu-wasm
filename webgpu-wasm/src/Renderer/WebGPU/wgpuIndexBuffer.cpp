#include "wgpuIndexBuffer.h"

#include "wgpuDevice.h"

WGpuIndexBuffer::WGpuIndexBuffer(WGpuDevice* device, const std::string& label, void* data, size_t size, IndexBufferFormat dataFormat)
: WGpuBuffer(device)
{
    WGpuBuffer::BufferDescription description{};
    description.label = label;
    description.bufferUsage = BufferUsage::Index;
    description.size = size;
    description.data = data;

    m_DataFormat = dataFormat;

    create(description);
}