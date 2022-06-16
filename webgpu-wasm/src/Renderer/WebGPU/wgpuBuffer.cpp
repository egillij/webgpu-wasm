#include "wgpuBuffer.h"

#include "wgpuDevice.h"

WGpuBuffer::WGpuBuffer(WGpuDevice* device)
: m_Device(device)
{

}

WGpuBuffer::WGpuBuffer(WGpuDevice* device, const BufferDescription& description)
: m_Device(device)
{
    create(description);
}

void WGpuBuffer::create(const BufferDescription& description)
{
    wgpu::BufferDescriptor descriptor{};
    descriptor.label = description.label.c_str();
    descriptor.usage = static_cast<wgpu::BufferUsage>(description.bufferUsage);
    descriptor.size = description.size;
    
    if(description.size > 0 && description.data){
        descriptor.mappedAtCreation = true;

        m_Buffer = m_Device->getHandle().CreateBuffer(&descriptor);
        memcpy(m_Buffer.GetMappedRange(), description.data, description.size);
        m_Buffer.Unmap();
    }
    else {
        m_Buffer = m_Device->getHandle().CreateBuffer(&descriptor);
    }
}