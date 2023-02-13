// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include "TextureUtils.h"

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuDevice;

class WGpuTexture {
public:
    WGpuTexture(const std::string& label, WGpuDevice* device);
    WGpuTexture(const std::string& label, const TextureCreateInfo* createInfo, WGpuDevice* device);
    ~WGpuTexture();

    inline wgpu::Texture& getHandle() { return m_Texture; }

    TextureFormat getFormat() const { return m_Format; }

    uint32_t getWidth() const { return m_Width; }
    uint32_t getHeight() const { return m_Height; }

    wgpu::TextureView createView();

    void update(const TextureCreateInfo* createInfo, WGpuDevice* device);

    const std::string& getLabel() const {return m_Label;}

private:
    std::string m_Label;
    wgpu::Texture m_Texture;

    TextureFormat m_Format;

    uint32_t m_Width, m_Height;
};