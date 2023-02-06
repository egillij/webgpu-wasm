#include "wgpuSampler.h"

#include "wgpuDevice.h"

WGpuSampler::WGpuSampler(const std::string& label, const SamplerCreateInfo* createInfo, WGpuDevice* device)
{
    wgpu::SamplerDescriptor descriptor{};
    descriptor.label = label.c_str();

    descriptor.addressModeU = static_cast<wgpu::AddressMode>(createInfo->addressModeU);
    descriptor.addressModeV = static_cast<wgpu::AddressMode>(createInfo->addressModeV);
    descriptor.addressModeW = static_cast<wgpu::AddressMode>(createInfo->addressModeW);

    descriptor.magFilter = static_cast<wgpu::FilterMode>(createInfo->magFilter);
    descriptor.minFilter = static_cast<wgpu::FilterMode>(createInfo->minFilter);
    descriptor.mipmapFilter = static_cast<wgpu::FilterMode>(createInfo->mipmapFilter);

    descriptor.lodMinClamp = createInfo->lodMinClamp;
    descriptor.lodMaxClamp = createInfo->lodMaxClamp;

    descriptor.compare = static_cast<wgpu::CompareFunction>(createInfo->compare);

    descriptor.maxAnisotropy = createInfo->maxAnisotropy;

    m_Sampler = device->getHandle().CreateSampler(&descriptor);
}

WGpuSampler::~WGpuSampler()
{
    m_Sampler.Release();
}