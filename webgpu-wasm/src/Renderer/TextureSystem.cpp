#include "TextureSystem.h"

#include "Renderer/WebGPU/wgpuDevice.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

static uint32_t nextId = 1;

TextureSystem::TextureSystem(WGpuDevice* device)
: m_Device(device)
{}

TextureSystem::~TextureSystem()
{}

WGpuTexture* TextureSystem::registerTexture(uint32_t id, const std::string& name, const void* data, uint32_t height, uint32_t width, TextureFormat format)
{
    //TODO: use id
    const auto it = m_Textures.find(nextId);
    if(it != m_Textures.end())
    {
        return m_Textures.at(nextId).get();
    }


    wgpu::Queue queue = m_Device->getHandle().GetQueue();
    TextureCreateInfo info{};
    info.format = format;
    info.height = height;
    info.width = width;
    info.usage = {TextureUsage::TextureBinding, TextureUsage::CopyDst};

    std::shared_ptr<WGpuTexture> texture = std::make_shared<WGpuTexture>(name, &info, m_Device);

    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = texture->getHandle();
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = 4;
    texDataLayout.rowsPerImage = 1;
    texDataLayout.offset = 0;

    wgpu::Extent3D texExtent{};
    texExtent.width = width;
    texExtent.height = height;

    size_t size = width * height;

    uint32_t elements = 1u;
    uint32_t elementSize = 1u;
    if(format == TextureFormat::BGRA8Unorm || format == TextureFormat::RGBA8Unorm){
        elements = 4u;
        elementSize = 1u;
    }
    if(format == TextureFormat::Depth32Float) {
        elements = 1u;
        elementSize = 4u;
    }

    size *= elements * elementSize;

    queue.WriteTexture(&imgCpyTex, data, size, &texDataLayout, &texExtent);

    m_Textures[nextId] = texture;

    return m_Textures.at(nextId++).get();
}

WGpuTexture* TextureSystem::registerTexture(uint32_t id, const std::string& name, const std::string& filename)
{
    //TODO: use id
    const auto it = m_Textures.find(nextId);
    if(it != m_Textures.end())
    {
        return m_Textures.at(nextId).get();
    }

    stbi_set_flip_vertically_on_load(true);
    int width, height, elements;
    unsigned char *imageData = stbi_load(filename.c_str(), &width, &height, &elements, 4);
    elements = 4;

    if(!imageData) {
        printf("Failed to load texture %s\n", filename.c_str());
        return nullptr;
    }

    wgpu::Queue queue = m_Device->getHandle().GetQueue();
    TextureCreateInfo info{};
    info.format = TextureFormat::RGBA8Unorm;
    info.height = height;
    info.width = width;
    info.usage = {TextureUsage::TextureBinding, TextureUsage::CopyDst};

    std::shared_ptr<WGpuTexture> texture = std::make_shared<WGpuTexture>(name, &info, m_Device);

    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = texture->getHandle();
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = width*elements;
    texDataLayout.rowsPerImage = height;
    texDataLayout.offset = 0;

    wgpu::Extent3D texExtent{};
    texExtent.width = width;
    texExtent.height = height;

    size_t size = width * height * elements;

    queue.WriteTexture(&imgCpyTex, imageData, size, &texDataLayout, &texExtent);

    m_Textures[nextId] = texture;

    return m_Textures.at(nextId++).get();
}

WGpuTexture* TextureSystem::find(uint32_t id)
{
    const auto it = m_Textures.find(id);
    if(it != m_Textures.end()) 
    {
        return m_Textures.at(id).get();
    }

    return nullptr;
}