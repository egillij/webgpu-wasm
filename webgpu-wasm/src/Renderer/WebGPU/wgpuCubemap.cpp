// Copyright 2023 Egill Ingi Jacobsen

#include "wgpuCubemap.h"

#include "wgpuDevice.h"

constexpr int CUBE_MAP_FACES = 6;

WGpuCubemap::WGpuCubemap(const std::string& label, WGpuDevice* device)
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
        texExtent.depthOrArrayLayers = CUBE_MAP_FACES;
        texDesc.size = texExtent;
        texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment;
    }

    m_Texture = device->getHandle().CreateTexture(&texDesc);

    for(int i = 0; i < CUBE_MAP_FACES; ++i){
        uint32_t data = 0xFFFFFFFF;
        if(i == 1){
            data = 0xFFFFFF00;
        }
        else if(i == 2){
            data = 0xFFFF0000;
        }
        else if(i == 3){
            data = 0xFF000000;
        }
        else if(i == 4){
            data = 0xFFFF00FF;
        }
        else if(i == 5){
            data = 0xFF00FFFF;
        }

        wgpu::ImageCopyTexture imgCpyTex{};
        imgCpyTex.texture = m_Texture;
        wgpu::Origin3D texOrig{};
        texOrig.z = i;
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
}

WGpuCubemap::WGpuCubemap(const std::string& label, const TextureCreateInfo* createInfo, WGpuDevice* device)
 : m_Label(""), m_Format(TextureFormat::Undefined), m_Width(0u), m_Height(0u)
{
    if(!createInfo) return;

    m_Label = label;
    m_Format = createInfo->format;
    m_Width = createInfo->width;
    m_Height = createInfo->height;

    wgpu::TextureDescriptor texDesc{};
    texDesc.label = label.c_str();
    texDesc.format = static_cast<wgpu::TextureFormat>(createInfo->format);
    wgpu::Extent3D texExtent{};
    texExtent.width = createInfo->width;
    texExtent.height = createInfo->height;
    texExtent.depthOrArrayLayers = CUBE_MAP_FACES;
    texDesc.size = texExtent;
    texDesc.usage = static_cast<wgpu::TextureUsage>(createInfo->usage[0]);
    for(int i = 1; i < createInfo->usage.size(); ++i){
        texDesc.usage = texDesc.usage | static_cast<wgpu::TextureUsage>(createInfo->usage[1]);
    }

    texDesc.dimension = wgpu::TextureDimension::e2D;

    m_Texture = device->getHandle().CreateTexture(&texDesc);
}

WGpuCubemap::~WGpuCubemap()
{
    m_Texture.Release();
}

wgpu::TextureView WGpuCubemap::createView(CubemapFace face)
{
    static uint64_t texViewNr = 0;
    wgpu::TextureViewDescriptor texViewDesc{};
    std::string viewLabel = m_Label + "_TexView_" + std::to_string(texViewNr);
    if(face == CubemapFace::ALL) {
        texViewDesc.label = viewLabel.c_str();
        texViewDesc.format = static_cast<wgpu::TextureFormat>(m_Format);
        texViewDesc.dimension = wgpu::TextureViewDimension::Cube;
        texViewDesc.mipLevelCount = 1;
        texViewDesc.arrayLayerCount = CUBE_MAP_FACES;
    }
    else {
        texViewDesc.label = viewLabel.c_str();
        texViewDesc.format = static_cast<wgpu::TextureFormat>(m_Format);
        texViewDesc.dimension = wgpu::TextureViewDimension::e2D;
        texViewDesc.mipLevelCount = 1;
        texViewDesc.baseArrayLayer = static_cast<uint32_t>(face);
        texViewDesc.arrayLayerCount = 1;
    }

    ++texViewNr;

    return m_Texture.CreateView(&texViewDesc);
}

void WGpuCubemap::update(const TextureCreateInfo* createInfo, WGpuDevice* device)
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
    texExtent.depthOrArrayLayers = CUBE_MAP_FACES;
    texDesc.size = texExtent;
    texDesc.usage = static_cast<wgpu::TextureUsage>(createInfo->usage[0]);
    for(int i = 1; i < createInfo->usage.size(); ++i){
        texDesc.usage = texDesc.usage | static_cast<wgpu::TextureUsage>(createInfo->usage[1]);
    }

    m_Texture = device->getHandle().CreateTexture(&texDesc);
}