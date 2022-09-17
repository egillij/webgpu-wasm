#include "wgpuBindGroup.h"

#include "wgpuDevice.h"
#include "wgpuBuffer.h"
#include "wgpuSampler.h"
#include "wgpuTexture.h"

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

void WGpuBindGroup::addBuffer(WGpuBuffer* buffer, BufferBindingType bindingType, uint64_t minBindingSize, uint32_t bindingSlot, wgpu::ShaderStage visiblity)
{
    Entry entry_;
    entry_.entryType = Entry::Type::Buffer;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visiblity;
    entry_.buffer.buffer = buffer;
    entry_.buffer.type = bindingType;
    entry_.buffer.minBindingSize = minBindingSize;

    m_Entries.push_back(entry_);
}

void WGpuBindGroup::addSampler(WGpuSampler* sampler, SamplerBindingType bindingType, uint32_t bindingSlot, wgpu::ShaderStage visibility)
{
    Entry entry_;
    entry_.entryType = Entry::Type::Sampler;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visibility;
    entry_.sampler.sampler = sampler;
    entry_.sampler.type = bindingType;

    m_Entries.push_back(entry_);
}

void WGpuBindGroup::addTexture(WGpuTexture* texture, TextureSampleType sampleType, uint32_t bindingSlot, wgpu::ShaderStage visibility)
{
    Entry entry_;
    entry_.entryType = Entry::Type::Texture;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visibility;
    entry_.texture.texture = texture;
    entry_.texture.type = sampleType;

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

        switch(entry_.entryType){
            case Entry::Type::Buffer: {
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
                break;
            }
            case Entry::Type::Sampler: {
                wgpu::SamplerBindingLayout sbl{};
                sbl.type = static_cast<wgpu::SamplerBindingType>(entry_.sampler.type);

                wgpu::BindGroupLayoutEntry bglEntry{};
                bglEntry.binding = entry_.bindingSlot;
                bglEntry.visibility = entry_.visibility;
                bglEntry.sampler = sbl;

                bindGroupLayoutEntries.emplace_back(bglEntry);

                wgpu::BindGroupEntry bge{};
                bge.binding = entry_.bindingSlot;
                bge.sampler = entry_.sampler.sampler->getHandle();

                bindGroupEntries.emplace_back(bge);

                break;
            }
            case Entry::Type::Texture: {
                wgpu::TextureBindingLayout tbl{};
                tbl.multisampled = false;
                tbl.sampleType = static_cast<wgpu::TextureSampleType>(entry_.texture.type);
                tbl.viewDimension = wgpu::TextureViewDimension::e2D;

                wgpu::BindGroupLayoutEntry bglEntry{};
                bglEntry.binding = entry_.bindingSlot;
                bglEntry.visibility = entry_.visibility;
                bglEntry.texture = tbl;

                bindGroupLayoutEntries.emplace_back(bglEntry);

                wgpu::BindGroupEntry bge{};
                bge.binding = entry_.bindingSlot;

                wgpu::TextureViewDescriptor texViewDesc{};
                texViewDesc.label = "Texture View Label"; //TODO: create something from the texture label??
                texViewDesc.format = wgpu::TextureFormat::RGBA8Unorm; //TODO: get from the texture?
                texViewDesc.dimension = wgpu::TextureViewDimension::e2D;
                texViewDesc.mipLevelCount = 1;
                texViewDesc.arrayLayerCount = 1;

                bge.textureView = entry_.texture.texture->getHandle().CreateView(&texViewDesc);

                bindGroupEntries.emplace_back(bge);

                break;
            }

        }
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