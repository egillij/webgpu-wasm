#include "wgpuTexture.h"

#include "wgpuDevice.h"

WGpuTexture::WGpuTexture(const std::string& label, const TextureCreateInfo* createInfo, WGpuDevice* device)
{
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