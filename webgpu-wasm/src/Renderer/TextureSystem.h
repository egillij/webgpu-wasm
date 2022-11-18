#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "Renderer/WebGPU/wgpuTexture.h"

class WGpuDevice;

class TextureSystem final {
public:
    TextureSystem(WGpuDevice* device);
    ~TextureSystem();

    WGpuTexture* registerTexture(uint32_t id, const std::string& name, const void* data, uint32_t height, uint32_t width, TextureFormat format);
    WGpuTexture* registerTexture(uint32_t id, const std::string& name, const std::string& filename);

    WGpuTexture* registerProceduralTexture(uint32_t id, const std::string& name, uint32_t height, uint32_t width, TextureFormat format);

    WGpuTexture* find(uint32_t id);

    void updateTexture(uint32_t id, void* data, int size);

private:
    WGpuDevice* m_Device;
    std::unordered_map<uint32_t, std::shared_ptr<WGpuTexture>> m_Textures;
};