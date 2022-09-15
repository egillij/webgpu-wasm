#include "wgpuBindGroup.h"

// #include "wgpuDevice.h"
// #include "wgpuBuffer.h"

// WGpuBindGroup::WGpuBindGroup()
// : m_Label("Default label Bind Group"), m_IsBuilt(false)
// {

// }

WGpuBindGroup::WGpuBindGroup(const std::string& label)
: m_Label(label)
{
    m_Entries.clear();
}
    
WGpuBindGroup::~WGpuBindGroup()
{

}

void WGpuBindGroup::addEntry(WGpuBuffer* buffer, BufferBindingType bindingType, uint64_t minBindingSize, uint32_t bindingSlot, wgpu::ShaderStage visiblity)
{
    Entry entry_;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visiblity;
    entry_.buffer.buffer = buffer;
    entry_.buffer.type = bindingType;
    entry_.buffer.minBindingSize = minBindingSize;

    m_Entries.push_back(entry_);
}

void WGpuBindGroup::build(WGpuDevice *device)
{
    if(!device) return;
    std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
    bindGroupLayoutEntries.reserve(m_Entries.size());

    std::vector<wgpu::BindGroupEntry> bindGroupEntries;
    bindGroupEntries.reserve(m_Entries.size());

    for(const auto& entry_ : m_Entries){
        wgpu::BufferBindingLayout bbl{};
        bbl.type = static_cast<wgpu::BufferBindingType>(entry_.buffer.type);
        bbl.minBindingSize = entry_.buffer.minBindingSize;

        wgpu::BindGroupLayoutEntry bglEntry{};
        bglEntry.binding = entry_.bindingSlot;
        bglEntry.visibility = entry_.visibility;
        bglEntry.buffer = bbl;

        bindGroupLayoutEntries.emplace_back(bglEntry);        

        wgpu::BindGroupEntry bge{};
        bge.binding = entry_.bindingSlot;
        bge.buffer = entry_.buffer.buffer->getHandle();
        bge.size = entry_.buffer.buffer->getSize();

        bindGroupEntries.emplace_back(bge);
    }

    {
        wgpu::BindGroupLayoutDescriptor descriptor{};
        descriptor.label = m_Label.c_str();
        descriptor.entryCount = bindGroupLayoutEntries.size();
        descriptor.entries = bindGroupLayoutEntries.data();

        m_Layout = device->getHandle().CreateBindGroupLayout(&descriptor);
    }
    
    {   
        wgpu::BindGroupDescriptor descriptor{};
        descriptor.label = m_Label.c_str();
        descriptor.entryCount = bindGroupEntries.size();
        descriptor.entries = bindGroupEntries.data();
        descriptor.layout = m_Layout;

        m_BindGroup = device->getHandle().CreateBindGroup(&descriptor);
    }
}
    
const wgpu::BindGroup& WGpuBindGroup::get() 
{
    return m_BindGroup;
}
    
wgpu::BindGroupLayout* WGpuBindGroup::getLayout() 
{
    return &m_Layout;
}