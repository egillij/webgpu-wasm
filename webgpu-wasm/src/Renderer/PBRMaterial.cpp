#include "Material.h"

#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"

#include "Renderer/TextureSystem.h"
#include "Application.h"

#include <webgpu/webgpu_cpp.h>
#include <emscripten.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb/stb_image.h>


PBRMaterial::PBRMaterial(const std::string &name, const glm::vec3 &color, WGpuDevice *device)
    : Material(name, MaterialType::PBR)
{
    m_Uniforms.shaderUniforms.albedo = color;
    m_Uniforms.shaderUniforms.ambient = glm::vec3(0.0f, 0.0f, 1.0f);
    m_Uniforms.shaderUniforms.specular = glm::vec3(1.f);
    m_Uniforms.shaderUniforms.shininess = 32.f;

    m_UniformBuffer = new WGpuUniformBuffer(device, m_Name + "_Material_UB", sizeof(PBRUniforms::ShaderUniforms));

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, &m_Uniforms.shaderUniforms, sizeof(PBRUniforms::ShaderUniforms));

    /////////////////////////////////
    // Load texture
    // emscripten_wget("/resources/textures/avatar.jpg", "./avatar.jpg");
    // stbi_set_flip_vertically_on_load(true);
    // int texwidth, texheight, texchannels;
    // unsigned char *imageData = stbi_load("./avatar.jpg", &texwidth, &texheight, &texchannels, 4);

    // wgpu::TextureDescriptor texDesc{};
    // texDesc.label = "Test texture";
    // texDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    // wgpu::Extent3D texExtent{};
    // texExtent.width = texwidth;
    // texExtent.height = texheight;
    // texDesc.size = texExtent;
    // texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;

    // TextureCreateInfo texInfo{};
    // texInfo.format = TextureFormat::RGBA8Unorm;
    // texInfo.width = texwidth;
    // texInfo.height = texheight;
    // texInfo.usage = {TextureUsage::CopyDst, TextureUsage::TextureBinding};

    // m_Texture = new WGpuTexture("Test texture", &texInfo, device);

    // SamplerCreateInfo samplerInfo{};
    // m_Sampler = new WGpuSampler("Sampler", &samplerInfo, device);
    // testTexture = wDevice.getHandle().CreateTexture(&texDesc);

    // wgpu::ImageCopyTexture imgCpyTex{};
    // imgCpyTex.texture = m_Texture->getHandle();
    // wgpu::Origin3D texOrig{};
    // imgCpyTex.origin = texOrig;

    // wgpu::TextureDataLayout texDataLayout{};
    // texDataLayout.bytesPerRow = texwidth * 4;
    // texDataLayout.rowsPerImage = texheight;
    // texDataLayout.offset = 0;

    // queue.WriteTexture(&imgCpyTex, imageData, texwidth * texheight * 4, &texDataLayout, &texExtent);

    m_MaterialBindGroupLayout = new WGpuBindGroupLayout(m_Name + "_Material_BGL");
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms), 0, wgpu::ShaderStage::Fragment);
    // m_MaterialBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    // m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(device);

    m_MaterialBindGroup = new WGpuBindGroup("Material Bind Group");
    m_MaterialBindGroup->setLayout(m_MaterialBindGroupLayout);
    m_MaterialBindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(PBRUniforms), 0, wgpu::ShaderStage::Fragment);
    // m_MaterialBindGroup->addTexture(m_Texture, TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->build(device);
}

PBRMaterial::PBRMaterial(const std::string& name, const PBRUniforms& data, WGpuDevice* device)
    : Material(name, MaterialType::PBR), m_AlbedoTexture(nullptr), m_AmbientTexture(nullptr), m_SpecularTexture(nullptr)
{
    m_Uniforms = data;

    m_UniformBuffer = new WGpuUniformBuffer(device, m_Name + "_Material_UB", sizeof(PBRUniforms::ShaderUniforms));

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, &m_Uniforms.shaderUniforms, sizeof(PBRUniforms::ShaderUniforms));

    // Create all textures

    if(data.textures.albedo.empty()){
        const unsigned char albedoData[4] = {
            (unsigned char) data.shaderUniforms.albedo[0],
            (unsigned char) data.shaderUniforms.albedo[1],
            (unsigned char) data.shaderUniforms.albedo[2],
            (unsigned char) 1
        };
        m_AlbedoTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_AlbedoTexture", &albedoData[0], 1, 1, TextureFormat::RGBA8Unorm);
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.albedo;
        std::string m_LocalResource = "./textures/" + data.textures.albedo;
        //TODO: gera async og ekki hér, heldur sem hluti af register material???
        emscripten_wget(m_ServerResource.c_str(), m_LocalResource.c_str());
        //TODO: load and create texture
        m_AlbedoTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_AlbedoTexture", m_LocalResource);
    }

    if(data.textures.ambient.empty()){
        const unsigned char ambientData[4] = {
            (unsigned char) data.shaderUniforms.ambient[0],
            (unsigned char) data.shaderUniforms.ambient[1],
            (unsigned char) data.shaderUniforms.ambient[2],
            (unsigned char) 1
        };
        m_AmbientTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_AmbientTexture", &ambientData[0], 1, 1, TextureFormat::RGBA8Unorm);
    }
    else {
        //TODO: load and create texture
    }

    if(data.textures.specular.empty()){
        const unsigned char specularData[4] = {
            (unsigned char) data.shaderUniforms.specular[0],
            (unsigned char) data.shaderUniforms.ambient[1],
            (unsigned char) data.shaderUniforms.ambient[2],
            (unsigned char) 1
        };
        m_SpecularTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_SpecularTexture", &specularData[0], 1, 1, TextureFormat::RGBA8Unorm);
    }
    else {
        //TODO: load and create texture
    }

    m_MaterialBindGroupLayout = new WGpuBindGroupLayout(m_Name + "_Material_BGL");
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(device);

    m_MaterialBindGroup = new WGpuBindGroup("Material Bind Group");
    m_MaterialBindGroup->setLayout(m_MaterialBindGroupLayout);
    m_MaterialBindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_AlbedoTexture, TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_AmbientTexture, TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_SpecularTexture, TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->build(device);
}

PBRMaterial::~PBRMaterial()
{
}