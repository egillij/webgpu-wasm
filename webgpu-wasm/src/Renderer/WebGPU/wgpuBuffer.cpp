// Copyright 2023 Egill Ingi Jacobsen

#include "wgpuBuffer.h"

#include "wgpuDevice.h"

WGpuBuffer::WGpuBuffer(WGpuDevice* device)
: m_Device(device)
{

}

WGpuBuffer::WGpuBuffer(WGpuDevice* device, const BufferDescription& description)
: m_Device(device), m_Buffer(nullptr)
{
    create(description);
}

WGpuBuffer::~WGpuBuffer()
{
    if(m_Buffer){
        m_Buffer.Destroy();
    }
}

void WGpuBuffer::create(const BufferDescription& description)
{
    m_Size = description.size;

    wgpu::BufferDescriptor descriptor{};
    descriptor.label = description.label.c_str();
    descriptor.usage = static_cast<wgpu::BufferUsage>(description.bufferUsage);
    descriptor.size = m_Size;

    if(description.size > 0 && description.data){
        descriptor.mappedAtCreation = true;

        m_Buffer = m_Device->getHandle().CreateBuffer(&descriptor);
        memcpy(m_Buffer.GetMappedRange(), description.data, description.size);
        m_Buffer.Unmap();
    }
    else {
        descriptor.mappedAtCreation = false;
        m_Buffer = m_Device->getHandle().CreateBuffer(&descriptor);
    }
}
