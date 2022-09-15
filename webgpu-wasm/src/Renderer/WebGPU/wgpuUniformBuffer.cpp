#include "wgpuUniformBuffer.h"

WGpuUniformBuffer::WGpuUniformBuffer(WGpuDevice* device, const std::string& label, uint64_t size)
: WGpuBuffer(device)
{
    WGpuBuffer::BufferDescription description{};
    description.label = label;
    description.bufferUsage = BufferUsage::Uniform | BufferUsage::CopyDst;
    description.size = size;
    description.data = nullptr;

    create(description);
}