#include "wgpuBindGroup.h"

#include "wgpuDevice.h"
#include "wgpuBuffer.h"
#include "wgpuSampler.h"
#include "wgpuTexture.h"

WGpuBindGroupLayout::WGpuBindGroupLayout(const std::string& label)
: m_Label(label)
{
    m_Entries.clear();
}
    
WGpuBindGroupLayout::~WGpuBindGroupLayout()
{

}

void WGpuBindGroupLayout::addBuffer(BufferBindingType bindingType, uint64_t minBindingSize, uint32_t bindingSlot, wgpu::ShaderStage visiblity)
{
    LayoutEntry entry_;
    entry_.entryType = LayoutEntry::Type::Buffer;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visiblity;
    entry_.buffer.type = bindingType;
    entry_.buffer.minBindingSize = minBindingSize;

    m_Entries.push_back(entry_);
}

void WGpuBindGroupLayout::addSampler(SamplerBindingType bindingType, uint32_t bindingSlot, wgpu::ShaderStage visibility)
{
    LayoutEntry entry_;
    entry_.entryType = LayoutEntry::Type::Sampler;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visibility;
    entry_.sampler.type = bindingType;

    m_Entries.push_back(entry_);
}

void WGpuBindGroupLayout::addTexture(TextureSampleType sampleType, uint32_t bindingSlot, wgpu::ShaderStage visibility)
{
    LayoutEntry entry_;
    entry_.entryType = LayoutEntry::Type::Texture;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visibility;
    entry_.texture.type = sampleType;

    m_Entries.push_back(entry_);
}

void WGpuBindGroupLayout::addStorageTexture(wgpu::StorageTextureAccess access, TextureFormat format, wgpu::TextureViewDimension dim, uint32_t bindingSlot, wgpu::ShaderStage visibility)
{
    LayoutEntry entry_;
    entry_.entryType = LayoutEntry::Type::StorageTexture;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visibility;

    entry_.storagTexture.access = access;
    entry_.storagTexture.format = format;
    entry_.storagTexture.dim = dim;

    m_Entries.push_back(entry_);
}

void WGpuBindGroupLayout::build(WGpuDevice *device)
{
    if(!device) return;
    std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
    bindGroupLayoutEntries.reserve(m_Entries.size());

    for(const auto& entry_ : m_Entries){

        switch(entry_.entryType){
            case LayoutEntry::Type::Buffer: {
                wgpu::BufferBindingLayout bbl{};
                bbl.type = static_cast<wgpu::BufferBindingType>(entry_.buffer.type);
                bbl.minBindingSize = entry_.buffer.minBindingSize;

                wgpu::BindGroupLayoutEntry bglEntry{};
                bglEntry.binding = entry_.bindingSlot;
                bglEntry.visibility = entry_.visibility;
                bglEntry.buffer = bbl;

                bindGroupLayoutEntries.emplace_back(bglEntry);        
                break;
            }
            case LayoutEntry::Type::Sampler: {
                wgpu::SamplerBindingLayout sbl{};
                sbl.type = static_cast<wgpu::SamplerBindingType>(entry_.sampler.type);

                wgpu::BindGroupLayoutEntry bglEntry{};
                bglEntry.binding = entry_.bindingSlot;
                bglEntry.visibility = entry_.visibility;
                bglEntry.sampler = sbl;

                bindGroupLayoutEntries.emplace_back(bglEntry);
                break;
            }
            case LayoutEntry::Type::Texture: {
                wgpu::TextureBindingLayout tbl{};
                tbl.multisampled = false;
                tbl.sampleType = static_cast<wgpu::TextureSampleType>(entry_.texture.type);
                tbl.viewDimension = wgpu::TextureViewDimension::e2D;

                wgpu::BindGroupLayoutEntry bglEntry{};
                bglEntry.binding = entry_.bindingSlot;
                bglEntry.visibility = entry_.visibility;
                bglEntry.texture = tbl;

                bindGroupLayoutEntries.emplace_back(bglEntry);
                break;
            }
            case LayoutEntry::Type::StorageTexture: {
                wgpu::StorageTextureBindingLayout stbl{};
                stbl.access = entry_.storagTexture.access; //TODO: nota eigin týpu
                stbl.format = static_cast<wgpu::TextureFormat>(entry_.storagTexture.format);
                stbl.viewDimension = entry_.storagTexture.dim; //TODO: nota eigin týpu

                wgpu::BindGroupLayoutEntry bglEntry{};
                bglEntry.binding = entry_.bindingSlot;
                bglEntry.visibility = entry_.visibility;
                bglEntry.storageTexture = stbl;

                bindGroupLayoutEntries.emplace_back(bglEntry);
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
}

wgpu::BindGroupLayout* WGpuBindGroupLayout::get()
{
    return &m_Layout;
}

// WgpuBindGroup

WGpuBindGroup::WGpuBindGroup(const std::string& label)
: m_Label(label)
{
    m_Layout = nullptr;
    m_Entries.clear();
}
    
WGpuBindGroup::~WGpuBindGroup()
{
    m_BindGroup.Release();
}

void WGpuBindGroup::setLayout(WGpuBindGroupLayout* layout)
{
    m_Layout = layout;
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

void WGpuBindGroup::addStorageTexture(WGpuTexture* texture, uint32_t bindingSlot, wgpu::ShaderStage visibility)
{
    Entry entry_;
    entry_.entryType = Entry::Type::StorageTexture;
    entry_.bindingSlot = bindingSlot;
    entry_.visibility = visibility;
    entry_.storageTexture.texture = texture;

    m_Entries.push_back(entry_);
}

void WGpuBindGroup::build(WGpuDevice *device)
{
    if(!device) return;

    std::vector<wgpu::BindGroupEntry> bindGroupEntries;
    bindGroupEntries.reserve(m_Entries.size());

    for(const auto& entry_ : m_Entries){

        switch(entry_.entryType){
            case Entry::Type::Buffer: {
                wgpu::BindGroupEntry bge{};
                bge.binding = entry_.bindingSlot;
                bge.buffer = entry_.buffer.buffer->getHandle();
                bge.size = entry_.buffer.buffer->getSize();

                bindGroupEntries.emplace_back(bge);
                break;
            }
            case Entry::Type::Sampler: {
                wgpu::BindGroupEntry bge{};
                bge.binding = entry_.bindingSlot;
                bge.sampler = entry_.sampler.sampler->getHandle();

                bindGroupEntries.emplace_back(bge);

                break;
            }
            case Entry::Type::Texture: {
                wgpu::BindGroupEntry bge{};
                bge.binding = entry_.bindingSlot;
                bge.textureView = entry_.texture.texture->createView();

                bindGroupEntries.emplace_back(bge);

                break;
            }
            case Entry::Type::StorageTexture: {
                wgpu::BindGroupEntry bge{};
                bge.binding = entry_.bindingSlot;
                bge.textureView = entry_.storageTexture.texture->createView();

                bindGroupEntries.emplace_back(bge);

                break;
            }

        }
    }
    
    {   
        wgpu::BindGroupDescriptor descriptor{};
        descriptor.label = m_Label.c_str();
        descriptor.entryCount = bindGroupEntries.size();
        descriptor.entries = bindGroupEntries.data();
        // TODO: assert here and report with error message in console
        if(!m_Layout) printf("Layout not present in %s\n", m_Label.c_str());
        descriptor.layout = *m_Layout->get();

        m_BindGroup = device->getHandle().CreateBindGroup(&descriptor);
    }
}
    
const wgpu::BindGroup& WGpuBindGroup::get() 
{
    return m_BindGroup;
}
    
WGpuBindGroupLayout* WGpuBindGroup::getLayout() 
{
    return m_Layout;
}