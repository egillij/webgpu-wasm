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

static WGpuTexture* createTexture(const std::string& name, const glm::vec4& color)
{
    const unsigned char texData[4] = {
        (unsigned char) (color[0]*255.f),
        (unsigned char) (color[1]*255.f),
        (unsigned char) (color[2]*255.f),
        (unsigned char) (color[3]*255.f)
    };
    return Application::get()->getTextureSystem()->registerTexture(0, name, &texData[0], 1, 1, TextureFormat::RGBA8Unorm);
}

PBRMaterial::PBRMaterial(const std::string& name, WGpuDevice* device)
: Material(name, MaterialType::PBR), m_AlbedoTexture(nullptr), m_MetallicTexture(nullptr),
  m_RoughnessTexture(nullptr), m_AoTexture(nullptr)
{
    m_Uniforms = PBRUniforms{};

    m_UniformBuffer = new WGpuUniformBuffer(device, m_Name + "_Material_UB", sizeof(PBRUniforms::ShaderUniforms));

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, &m_Uniforms.shaderUniforms, sizeof(PBRUniforms::ShaderUniforms));

    // Create all textures
    m_AlbedoTexture = createTexture(m_Name + "_AlbedoTexture", glm::vec4(m_Uniforms.shaderUniforms.albedo, 1.f));

    //TODO: use only a single channel texture to save space
    m_MetallicTexture = createTexture(m_Name + "_MetallicTexture", glm::vec4(m_Uniforms.shaderUniforms.metallic));

    //TODO: use only a single channel texture to save space
    m_RoughnessTexture = createTexture(m_Name + "_RoughnessTexture", glm::vec4(m_Uniforms.shaderUniforms.roughness));

    //TODO: use only a single channel texture to save space
    m_AoTexture = createTexture(m_Name + "_AOTexture", glm::vec4(m_Uniforms.shaderUniforms.ambientOcclusions));
    

    m_MaterialBindGroupLayout = new WGpuBindGroupLayout(m_Name + "_Material_BGL");
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 4, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(device);

    m_MaterialBindGroup = new WGpuBindGroup("Material Bind Group");
    m_MaterialBindGroup->setLayout(m_MaterialBindGroupLayout);
    m_MaterialBindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_AlbedoTexture, TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_MetallicTexture, TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_RoughnessTexture, TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_AoTexture, TextureSampleType::Float, 4, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->build(device);

}

PBRMaterial::PBRMaterial(const std::string& name, const PBRUniforms& data, WGpuDevice* device)
    : Material(name, MaterialType::PBR), m_AlbedoTexture(nullptr), m_MetallicTexture(nullptr), 
    m_RoughnessTexture(nullptr), m_AoTexture(nullptr)
{
    m_Uniforms = data;

    m_UniformBuffer = new WGpuUniformBuffer(device, m_Name + "_Material_UB", sizeof(PBRUniforms::ShaderUniforms));

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, &m_Uniforms.shaderUniforms, sizeof(PBRUniforms::ShaderUniforms));

    // Create all textures

    if(data.textures.albedo.empty()){
        m_AlbedoTexture = createTexture(m_Name + "_AlbedoTexture", glm::vec4(data.shaderUniforms.albedo, 1.f));
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.albedo;
        m_AlbedoTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_AlbedoTexture", m_ServerResource);
    }

    if(data.textures.metallic.empty()){
        //TODO: use only a single channel texture to save space
        m_MetallicTexture = createTexture(m_Name + "_MetallicTexture", glm::vec4(data.shaderUniforms.metallic));
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.metallic;
        m_MetallicTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_MetallicTexture", m_ServerResource);
    }

    if(data.textures.roughness.empty()){
        //TODO: use only a single channel texture to save space
        m_RoughnessTexture = createTexture(m_Name + "_RoughnessTexture", glm::vec4(data.shaderUniforms.roughness));
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.roughness;
        m_RoughnessTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_RoughnessTexture", m_ServerResource);
    }

    if(data.textures.ambientOcclusion.empty()){
        //TODO: use only a single channel texture to save space
        m_AoTexture = createTexture(m_Name + "_AOTexture", glm::vec4(data.shaderUniforms.ambientOcclusions));
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.ambientOcclusion;
        m_AoTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_AOTexture", m_ServerResource);
    }

    
    m_MaterialBindGroupLayout = new WGpuBindGroupLayout(m_Name + "_Material_BGL");
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 4, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(device);

    m_MaterialBindGroup = new WGpuBindGroup("Material Bind Group");
    m_MaterialBindGroup->setLayout(m_MaterialBindGroupLayout);
    m_MaterialBindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_AlbedoTexture, TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_MetallicTexture, TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_RoughnessTexture, TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_AoTexture, TextureSampleType::Float, 4, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->build(device);
}

PBRMaterial::~PBRMaterial()
{
}

void PBRMaterial::update(const PBRUniforms& data, WGpuDevice* device)
{
    m_Uniforms = data;

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, &m_Uniforms.shaderUniforms, sizeof(PBRUniforms::ShaderUniforms));

    // Create all textures
    // TODO: instead of overwriting textures here we should be updating the existing ones
    if(data.textures.albedo.empty()){
        m_AlbedoTexture = createTexture(m_Name + "_AlbedoTexture", glm::vec4(data.shaderUniforms.albedo, 1.f));
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.albedo;
        m_AlbedoTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_AlbedoTexture", m_ServerResource);
    }

    if(data.textures.metallic.empty()){
        //TODO: use only a single channel texture to save space
        m_MetallicTexture = createTexture(m_Name + "_MetallicTexture", glm::vec4(data.shaderUniforms.metallic));
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.metallic;
        m_MetallicTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_MetallicTexture", m_ServerResource);
    }

    if(data.textures.roughness.empty()){
        //TODO: use only a single channel texture to save space
        m_RoughnessTexture = createTexture(m_Name + "_RoughnessTexture", glm::vec4(data.shaderUniforms.roughness));
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.roughness;
        m_RoughnessTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_RoughnessTexture", m_ServerResource);
    }

    if(data.textures.ambientOcclusion.empty()){
        //TODO: use only a single channel texture to save space
        m_AoTexture = createTexture(m_Name + "_AOTexture", glm::vec4(data.shaderUniforms.ambientOcclusions));
    }
    else {
        // TODO: textures should be referenced in the scene description so they can be loaded/created up front?
        std::string m_ServerResource = "/resources/textures/" + data.textures.ambientOcclusion;
        m_AoTexture = Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_AOTexture", m_ServerResource);
    }

    
    updateBindGroup(device);
}

void PBRMaterial::updateBindGroup(WGpuDevice* device) 
{
    if(m_MaterialBindGroup) delete m_MaterialBindGroup;

    m_MaterialBindGroup = new WGpuBindGroup("Material Bind Group");
    m_MaterialBindGroup->setLayout(m_MaterialBindGroupLayout);
    m_MaterialBindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_AlbedoTexture, TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_MetallicTexture, TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_RoughnessTexture, TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->addTexture(m_AoTexture, TextureSampleType::Float, 4, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroup->build(device);
}