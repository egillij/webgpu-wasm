#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuDevice;

enum class TextureFormat : uint32_t {
    BGRA8Unorm = static_cast<uint32_t>(wgpu::TextureFormat::BGRA8Unorm),
    RGBA8Unorm = static_cast<uint32_t>(wgpu::TextureFormat::RGBA8Unorm),
    Depth32Float = static_cast<uint32_t>(wgpu::TextureFormat::Depth32Float),
};

enum class TextureUsage : uint32_t {
    None = static_cast<uint32_t>(wgpu::TextureUsage::None),
    CopySrc = static_cast<uint32_t>(wgpu::TextureUsage::CopySrc),
    CopyDst = static_cast<uint32_t>(wgpu::TextureUsage::CopyDst),
    TextureBinding = static_cast<uint32_t>(wgpu::TextureUsage::TextureBinding),
    StorageBinding = static_cast<uint32_t>(wgpu::TextureUsage::StorageBinding),
    RenderAttachment = static_cast<uint32_t>(wgpu::TextureUsage::RenderAttachment),
};

struct TextureCreateInfo {
  TextureFormat format;
  uint32_t width;
  uint32_t height;
  std::vector<TextureUsage> usage;  
};

class WGpuTexture {
public:
    WGpuTexture(const std::string& label, const TextureCreateInfo* createInfo, WGpuDevice* device);
    ~WGpuTexture();

    inline wgpu::Texture& getHandle() { return m_Texture; }

private:
    wgpu::Texture m_Texture;
};