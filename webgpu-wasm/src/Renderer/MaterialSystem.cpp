#include "MaterialSystem.h"

#include "Renderer/WebGPU/wgpuDevice.h"

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