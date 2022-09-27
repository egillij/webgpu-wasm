#include "wgpuIndexBuffer.h"

#include "wgpuDevice.h"

WGpuIndexBuffer::WGpuIndexBuffer(WGpuDevice* device, const std::string& label, void* data, uint64_t indexCount, IndexBufferFormat dataFormat)
: WGpuBuffer(device)
{
    WGpuBuffer::BufferDescription description{};
    description.label = label;
    description.bufferUsage = BufferUsage::Index;
    description.size = indexCount*sizeof(uint32_t);
    description.data = data;

    m_DataFormat = dataFormat;

    m_IndexCount = indexCount;

    create(description);
}