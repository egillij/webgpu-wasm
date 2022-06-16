#include "wgpuUniformBuffer.h"

WGpuUniformBuffer::WGpuUniformBuffer(WGpuDevice* device, const std::string& label, uint64_t size)
: WGpuBuffer(device)
{
    WGpuBuffer::BufferDescription description{};
    description.label = label;
    description.bufferUsage = BufferUsage::Uniform | BufferUsage::CopyDst;
    description.size = size;
    description.data = nullptr;

    printf("a: %u\n", static_cast<uint32_t>(BufferUsage::Uniform));
    printf("b: %u\n", static_cast<uint32_t>(BufferUsage::CopyDst));
    printf("a|b = %u\n", static_cast<uint32_t>(description.bufferUsage));

    create(description);
}