#include "Material.h"

#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"

#include <webgpu/webgpu_cpp.h>
#include <emscripten.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

PBRMaterial::PBRMaterial(const std::string &name, const glm::vec3 &color, WGpuDevice *device)
    : Material(name, MaterialType::PBR)
{
    m_Uniforms.albedo = color;
    m_Uniforms.ambient = glm::vec3(0.0f, 0.0f, 1.0f);
    m_Uniforms.specular = glm::vec3(1.f);
    m_Uniforms.shininess = 32.f;

    m_UniformBuffer = new WGpuUniformBuffer(device, m_Name + "_Material_UB", sizeof(PBRUniforms));

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, &m_Uniforms, sizeof(PBRUniforms));

    /////////////////////////////////
    // Load texture
    emscripten_wget("/resources/textures/avatar.jpg", "./avatar.jpg");
    stbi_set_flip_vertically_on_load(true);
    int texwidth, texheight, texchannels;
    unsigned char *imageData = stbi_load("./avatar.jpg", &texwidth, &texheight, &texchannels, 4);

    // wgpu::TextureDescriptor texDesc{};
    // texDesc.label = "Test texture";
    // texDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::Extent3D texExtent{};
    texExtent.width = texwidth;
    texExtent.height = texheight;
    // texDesc.size = texExtent;
    // texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;

    TextureCreateInfo texInfo{};
    texInfo.format = TextureFormat::RGBA8Unorm;
    texInfo.width = texwidth;
    texInfo.height = texheight;
    texInfo.usage = {TextureUsage::CopyDst, TextureUsage::TextureBinding};

    m_Texture = new WGpuTexture("Test texture", &texInfo, device);

    SamplerCreateInfo samplerInfo{};
    m_Sampler = new WGpuSampler("Sampler", &samplerInfo, device);
    // testTexture = wDevice.getHandle().CreateTexture(&texDesc);

    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = m_Texture->getHandle();
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = texwidth * 4;
    texDataLayout.rowsPerImage = texheight;
    texDataLayout.offset = 0;

    queue.WriteTexture(&imgCpyTex, imageData, texwidth * texheight * 4, &texDataLayout, &texExtent);

    m_MaterialBindGroupLayout = new WGpuBindGroupLayout(m_Name + "_Material_BGL");
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms), 0, wgpu::ShaderStage::Fragment);
    // m_MaterialBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    // m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(device);

    m_MaterialBindGroup = new WGpuBindGroup("Material Bind Group");
    m_MaterialBindGroup->setLayout(m_MaterialBindGroupLayout);
    m_MaterialBindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(PBRUniforms), 0, wgpu::ShaderStage::Fragment);
    // m_MaterialBindGroup->addSampler(m_Sampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    // m_MaterialBindGroup->addTexture(m_Texture, TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->build(device);
}

PBRMaterial::PBRMaterial(const std::string& name, const PBRUniforms& data, WGpuDevice* device)
    : Material(name, MaterialType::PBR)
{
    m_Uniforms = data;

    m_UniformBuffer = new WGpuUniformBuffer(device, m_Name + "_Material_UB", sizeof(PBRUniforms));

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, &m_Uniforms, sizeof(PBRUniforms));

    m_MaterialBindGroupLayout = new WGpuBindGroupLayout(m_Name + "_Material_BGL");
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(device);

    m_MaterialBindGroup = new WGpuBindGroup("Material Bind Group");
    m_MaterialBindGroup->setLayout(m_MaterialBindGroupLayout);
    m_MaterialBindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(PBRUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->build(device);
}

PBRMaterial::~PBRMaterial()
{
}