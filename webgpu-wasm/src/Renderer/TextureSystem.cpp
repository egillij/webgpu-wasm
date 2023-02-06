#include "TextureSystem.h"

#include "Tasks/ITask.h"

#include "Application.h"

#include "Renderer/WebGPU/wgpuDevice.h"

#include "Renderer/MaterialSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <emscripten.h>

struct LoadData {
    TextureSystem* texSystem;
    uint32_t textureId;
    TexDataType expectedType;
    TextureLoadTask* task = nullptr;
};

std::string extractExtension(const std::string& filename)
{
    size_t lastDot = filename.find_last_of(".");
    std::string extension = filename.substr(lastDot+1, filename.size()-lastDot-1);

    return extension;
}

void onTextureLoadSuccess(void* userData, void* data, int size)
{
    if(!userData)
    {
        printf("Texture system lost. Can't update texture!\n");
        return;
    }

    LoadData* loadData = (LoadData*) userData;
    loadData->texSystem->updateTexture(loadData->textureId, data, size, loadData->expectedType, loadData->task);

    if(loadData->task)
        delete loadData->task;

    delete loadData;
}

void onTextureLoadError(void* userData) 
{
    if(!userData){
        printf("Failed to load texture. Texture system lost\n");
    }
    else {
        LoadData* loadData = (LoadData*)userData;
        printf("Failed to load texture %u\n", loadData->textureId);
        if(loadData->task) delete loadData->task;
        delete loadData;
    }
}

void onCubemapLoadSuccess(void* userData, void* data, int size)
{
    if(!userData)
    {
        printf("Texture system lost. Can't update cubemap texture!\n");
        return;
    }

    LoadData* loadData = (LoadData*) userData;
    loadData->texSystem->updateCubemap(loadData->textureId, data, size);

    delete loadData;
}

void onCubemapLoadError(void* userData) 
{
    if(!userData){
        printf("Failed to load cubemap texture. Texture system lost\n");
    }
    else {
        LoadData* loadData = (LoadData*)userData;
        printf("Failed to load texture %u\n", loadData->textureId);
        delete loadData;
    }
}

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

WGpuTexture* TextureSystem::registerTexture(uint32_t id, const std::string& name, const std::string& filename, TextureLoadTask* loadTask)
{
    //TODO: use id
    id = nextId++;

    const auto it = m_Textures.find(id);
    if(it != m_Textures.end())
    {
        return m_Textures.at(id).get();
    }

    std::shared_ptr<WGpuTexture> texture = std::make_shared<WGpuTexture>(name, m_Device);
    m_Textures[id] = texture;

    LoadData* loadData = new LoadData;
    loadData->texSystem = this;
    loadData->textureId = id;
    loadData->task = loadTask;

    std::string extension = extractExtension(filename);
    if(extension == "hdr"){
        loadData->expectedType = TexDataType::Float;
    }
    else {
        loadData->expectedType = TexDataType::UnsignedByte;
    }

    emscripten_async_wget_data(filename.c_str(), (void*)loadData, onTextureLoadSuccess, onTextureLoadError);

    return m_Textures.at(id).get();
}

WGpuTexture* TextureSystem::registerProceduralTexture(uint32_t id, const std::string& name, uint32_t height, uint32_t width, TextureFormat format)
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
    info.usage = {TextureUsage::TextureBinding, TextureUsage::StorageBinding};
    std::shared_ptr<WGpuTexture> texture = std::make_shared<WGpuTexture>(name, &info, m_Device);

    m_Textures[nextId] = texture;

    return m_Textures.at(nextId++).get();
}

WGpuCubemap* TextureSystem::registerCubemap(uint32_t id, const std::string& name, const std::string& filename)
{
    //TODO: use id
    id = nextId++;

    const auto it = m_Cubemaps.find(id);
    if(it != m_Cubemaps.end())
    {
        return m_Cubemaps.at(id).get();
    }

    std::shared_ptr<WGpuTexture> texture = std::make_shared<WGpuTexture>(name, m_Device);
    m_Textures[id] = texture;

    LoadData* loadData = new LoadData;
    loadData->texSystem = this;
    loadData->textureId = id;
    loadData->task = nullptr;

    emscripten_async_wget_data(filename.c_str(), (void*)loadData, onCubemapLoadSuccess, onCubemapLoadError);

    return m_Cubemaps.at(id).get();
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

void TextureSystem::updateTexture(uint32_t id, void* data, int size, TexDataType dataType, TextureLoadTask* loadTask)
{
    const auto it = m_Textures.find(id);
    if(it == m_Textures.end()) 
    {
        printf("Could not find texture with id %u. No update done.\n", id);
        return;
    }

    auto texture = it->second;

    wgpu::Queue queue = m_Device->getHandle().GetQueue();
    TextureCreateInfo info{};

    int width, height, elements;
    void* imageData;
    size_t elementSize = 0;
    if(dataType == TexDataType::Float){
        info.format = TextureFormat::RGBA32Float;
        elementSize = sizeof(float);
        stbi_set_flip_vertically_on_load(true);
        imageData = stbi_loadf_from_memory((unsigned char*)data, size, &width, &height, &elements, 4);
        elements = 4;

        if(!imageData) {
            printf("Failed to load texture from memory. No update done.\n");
            return;
        }
    }
    else {
        info.format = TextureFormat::RGBA8Unorm;
        elementSize = sizeof(char);
        stbi_set_flip_vertically_on_load(true);
        imageData = stbi_load_from_memory((unsigned char*)data, size, &width, &height, &elements, 4);
        elements = 4;

        if(!imageData) {
            printf("Failed to load texture from memory. No update done.\n");
            return;
        }
    }

    info.height = height;
    info.width = width;
    info.usage = {TextureUsage::TextureBinding, TextureUsage::CopyDst};

    texture->update(&info, m_Device);

    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = texture->getHandle();
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = width*elements*elementSize;
    texDataLayout.rowsPerImage = height;
    texDataLayout.offset = 0;

    wgpu::Extent3D texExtent{};
    texExtent.width = width;
    texExtent.height = height;

    size_t texSize = width * height * elements * elementSize;

    queue.WriteTexture(&imgCpyTex, imageData, texSize, &texDataLayout, &texExtent);

    // TODO: force an update any materials using this textur instead of updating everything. Do this with events?
    Application::get()->getMaterialSystem()->updateBindgroups();

    if(loadTask) {
        loadTask->execute(texture.get());
    }
}

void TextureSystem::updateCubemap(uint32_t id, void* data, int size)
{
    const auto it = m_Cubemaps.find(id);
    if(it == m_Cubemaps.end()) 
    {
        printf("Could not find cubemap with id %u. No update done.\n", id);
        return;
    }

    auto cubemap = it->second;

    stbi_set_flip_vertically_on_load(true);
    int width, height, elements;
    unsigned char *imageData = stbi_load_from_memory((unsigned char*)data, size, &width, &height, &elements, 4);
    elements = 4;

    if(!imageData) {
        printf("Failed to load texture from memory. No update done.\n");
        return;
    }

    wgpu::Queue queue = m_Device->getHandle().GetQueue();
    TextureCreateInfo info{};
    info.format = TextureFormat::RGBA8Unorm;
    info.height = height;
    info.width = width;
    info.usage = {TextureUsage::TextureBinding, TextureUsage::CopyDst};

    WGpuTexture texture = WGpuTexture(cubemap->getLabel() + "_RawTexture", &info, m_Device);
    texture.update(&info, m_Device);

    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = texture.getHandle();
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = width*elements;
    texDataLayout.rowsPerImage = height;
    texDataLayout.offset = 0;

    wgpu::Extent3D texExtent{};
    texExtent.width = width;
    texExtent.height = height;

    size_t texSize = width * height * elements;

    queue.WriteTexture(&imgCpyTex, imageData, texSize, &texDataLayout, &texExtent);

    //TODO: run a pipeline to render a cubemap from the texture

    // TODO: force an update any materials using this texture instead of updating everything. Do this with events?
    Application::get()->getMaterialSystem()->updateBindgroups();
}

void TextureSystem::clear()
{
    m_Textures.clear();
}