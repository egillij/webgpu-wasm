#include "wgpuVertexBuffer.h"

#include "wgpuBuffer.h"

#include "wgpuDevice.h"

WGpuVertexBuffer::WGpuVertexBuffer(WGpuDevice* device, const std::string& label, void* data, size_t size)
: WGpuBuffer(device)
{
    WGpuBuffer::BufferDescription description{};
    description.label = label;
    description.bufferUsage = BufferUsage::Vertex;
    description.size = size;
    description.data = data;

    create(description);
}

WGpuVertexBuffer::~WGpuVertexBuffer() 
{
}