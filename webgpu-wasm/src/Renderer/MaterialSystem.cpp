#include "MaterialSystem.h"

#include "Renderer/WebGPU/wgpuDevice.h"

#define MATS_IO_LOAD
#include "matsIo.h"

MaterialSystem::MaterialSystem(WGpuDevice* device)
: m_Device(device)
{
}

MaterialSystem::~MaterialSystem()
{
}

Material* MaterialSystem::registerMaterial(uint32_t id, const std::string& name, const PBRUniforms& data)
{
    const auto it = m_Materials.find(id);
    if(it == m_Materials.end()){
        m_Materials[id] = std::make_shared<PBRMaterial>(name, data, m_Device);
    }

    return m_Materials.at(id).get();
}

Material* MaterialSystem::registerMaterial(uint32_t id, const std::string& name, const std::string& filename)
{
    const auto it = m_Materials.find(id);
    if(it != m_Materials.end()){
        return m_Materials.at(id).get();
    }

    mats::MatsIO io = mats::MatsIO();

    mats::PhongMaterial data{};
    bool success = io.load(filename.c_str(), &data);

    PBRUniforms materialData{};
    if(success){
        materialData.albedo = data.albedo;
        materialData.ambient = data.ambient;
        materialData.specular = data.specular;
        materialData.shininess = data.shininess;
    }
    
    m_Materials[id] = std::make_shared<PBRMaterial>(name, materialData, m_Device);

    return m_Materials.at(id).get();
}

Material* MaterialSystem::find(uint32_t id)
{
    const auto it = m_Materials.find(id);
    if(it != m_Materials.end()){
        return m_Materials.at(id).get();
    }
    return nullptr;
}