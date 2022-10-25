#include "wgpuTexture.h"

#include "wgpuDevice.h"

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
    texDesc.size = texExtent;
    texDesc.usage = static_cast<wgpu::TextureUsage>(createInfo->usage[0]);
    for(int i = 1; i < createInfo->usage.size(); ++i){
        texDesc.usage = texDesc.usage | static_cast<wgpu::TextureUsage>(createInfo->usage[1]);
    }

    m_Texture = device->getHandle().CreateTexture(&texDesc);
}

WGpuTexture::~WGpuTexture()
{

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