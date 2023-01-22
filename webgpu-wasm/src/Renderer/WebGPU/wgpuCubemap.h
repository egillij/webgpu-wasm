#pragma once

#include "TextureUtils.h"

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuDevice;

enum class CubemapFace : uint32_t {
    RIGHT = 0, LEFT, TOP, BOTTOM, BACK, FRONT, ALL
};

class WGpuCubemap {
public:
    WGpuCubemap(const std::string& label, WGpuDevice* device);
    WGpuCubemap(const std::string& label, const TextureCreateInfo* createInfo, WGpuDevice* device);
    ~WGpuCubemap();

    inline wgpu::Texture& getHandle() { return m_Texture; }

    TextureFormat getFormat() const { return m_Format; }

    uint32_t getWidth() const { return m_Width; }
    uint32_t getHeight() const { return m_Height; }

    wgpu::TextureView createView(CubemapFace face = CubemapFace::ALL);

    void update(const TextureCreateInfo* createInfo, WGpuDevice* device);

    const std::string& getLabel() const {return m_Label;}

private:
    std::string m_Label;
    wgpu::Texture m_Texture;

    TextureFormat m_Format;

    uint32_t m_Width, m_Height;
};