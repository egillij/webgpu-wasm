#include "wgpuTexture.h"

#include "wgpuDevice.h"

WGpuTexture::WGpuTexture(const std::string& label, WGpuDevice* device)
 : m_Label(label), m_Format(TextureFormat::Undefined), m_Width(0u), m_Height(0u)
{
    // Create default 1 by 1 texture here
    m_Format = TextureFormat::RGBA8Unorm;

    wgpu::TextureDescriptor texDesc{};
    {
        texDesc.label = label.c_str();
        texDesc.format = static_cast<wgpu::TextureFormat>(m_Format);
        wgpu::Extent3D texExtent{};
        texExtent.width = 1;
        texExtent.height = 1;
        texDesc.size = texExtent;
        texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    }

    m_Texture = device->getHandle().CreateTexture(&texDesc);

    uint32_t data = 0xFF0000FF;

    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = m_Texture;
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = 4;
    texDataLayout.rowsPerImage = 1;
    texDataLayout.offset = 0;

    wgpu::Extent3D texExtent{};
    texExtent.width = 1;
    texExtent.height = 1;

    size_t size = 1;

    uint32_t elements = 4u;
    uint32_t elementSize = 1u;

    size *= elements * elementSize;

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteTexture(&imgCpyTex, &data, size, &texDataLayout, &texExtent);
}

WGpuTexture::WGpuTexture(const std::string& label, const TextureCreateInfo* createInfo, WGpuDevice* device)
 : m_Label(""), m_Format(TextureFormat::Undefined), m_Width(0u), m_Height(0u)
{
    if(!createInfo) return;

    m_Label = label;
    m_Format = createInfo->format;

    wgpu::TextureDescriptor texDesc{};
    texDesc.label = label.c_str();
    texDesc.format = static_cast<wgpu::TextureFormat>(createInfo->format);
    wgpu::Extent3D texExtent{};
    texExtent.width = createInfo->width;
    texExtent.height = createInfo->height;
    texExtent.depthOrArrayLayers = 1u;
    texDesc.size = texExtent;
    texDesc.usage = static_cast<wgpu::TextureUsage>(createInfo->usage[0]);
    for(int i = 1; i < createInfo->usage.size(); ++i){
        texDesc.usage = texDesc.usage | static_cast<wgpu::TextureUsage>(createInfo->usage[1]);
    }

    texDesc.dimension = wgpu::TextureDimension::e2D;

    m_Texture = device->getHandle().CreateTexture(&texDesc);
}

WGpuTexture::~WGpuTexture()
{
    m_Texture.Release();
}

wgpu::TextureView WGpuTexture::createView()
{
    static uint64_t texViewNr = 0;
    wgpu::TextureViewDescriptor texViewDesc{};
    std::string viewLabel = m_Label + "_TexView_" + std::to_string(texViewNr);
    texViewDesc.label = viewLabel.c_str();
    texViewDesc.format = static_cast<wgpu::TextureFormat>(m_Format);
    texViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    texViewDesc.mipLevelCount = 1;
    texViewDesc.arrayLayerCount = 1;

    ++texViewNr;

    return m_Texture.CreateView(&texViewDesc);
}

void WGpuTexture::update(const TextureCreateInfo* createInfo, WGpuDevice* device)
{
    if(!createInfo || !device) return;
    
    m_Texture.Release();
    m_Texture = nullptr;

    m_Format = createInfo->format;

    wgpu::TextureDescriptor texDesc{};
    texDesc.label = m_Label.c_str();
    texDesc.format = static_cast<wgpu::TextureFormat>(createInfo->format);
    wgpu::Extent3D texExtent{};
    texExtent.width = createInfo->width;
    texExtent.height = createInfo->height;
    texExtent.depthOrArrayLayers = 1u;
    texDesc.size = texExtent;
    texDesc.usage = static_cast<wgpu::TextureUsage>(createInfo->usage[0]);
    for(int i = 1; i < createInfo->usage.size(); ++i){
        texDesc.usage = texDesc.usage | static_cast<wgpu::TextureUsage>(createInfo->usage[1]);
    }

    m_Texture = device->getHandle().CreateTexture(&texDesc);
}