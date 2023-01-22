#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuCubemap.h"

class WGpuDevice;

class TextureLoadTask {
public:
    virtual ~TextureLoadTask() = default;
    virtual void execute(WGpuTexture* texture) = 0;
// protected:
//     TextureLoadTask();    
};

class TextureSystem final {
public:
    TextureSystem(WGpuDevice* device);
    ~TextureSystem();

    WGpuTexture* registerTexture(uint32_t id, const std::string& name, const void* data, uint32_t height, uint32_t width, TextureFormat format);
    WGpuTexture* registerTexture(uint32_t id, const std::string& name, const std::string& filename, TextureLoadTask* loadTask = nullptr);

    WGpuTexture* registerProceduralTexture(uint32_t id, const std::string& name, uint32_t height, uint32_t width, TextureFormat format);

    WGpuCubemap* registerCubemap(uint32_t id, const std::string& name, const std::string& filename);

    WGpuTexture* find(uint32_t id);

    void updateTexture(uint32_t id, void* data, int size, TextureLoadTask* loadTask = nullptr);
    void updateCubemap(uint32_t id, void* data, int size);

    void clear();

private:
    WGpuDevice* m_Device;
    std::unordered_map<uint32_t, std::shared_ptr<WGpuTexture>> m_Textures;
    std::unordered_map<uint32_t, std::shared_ptr<WGpuCubemap>> m_Cubemaps;
};