#include "MaterialSystem.h"

#include "Renderer/WebGPU/wgpuDevice.h"

#define MATS_IO_LOAD
#include "matsIo.h"

#include <emscripten.h>

struct LoadData {
    MaterialSystem* matSystem;
    uint32_t materialId;
};

void onMaterialLoadSuccess(void* userData, void* data, int size)
{
    if(!userData)
    {
        printf("Material system lost. Can't update material!\n");
        return;
    }

    LoadData* loadData = (LoadData*) userData;
    loadData->matSystem->updateMaterial(loadData->materialId, data, size);

    delete loadData;
}

void onMaterialLoadError(void* userData) 
{
    if(!userData){
        printf("Failed to load material. Material system lost\n");
    }
    else {
        LoadData* loadData = (LoadData*)userData;
        printf("Failed to load material %u\n", loadData->materialId);
        delete loadData;
    }
}

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
    
    m_Materials[id] = std::make_shared<PBRMaterial>(name, m_Device);

    LoadData* loadData = new LoadData;
    loadData->matSystem = this;
    loadData->materialId = id;

    emscripten_async_wget_data(filename.c_str(), (void*)loadData, onMaterialLoadSuccess, onMaterialLoadError);

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

void MaterialSystem::updateMaterial(uint32_t id, void* data, int size)
{
    const auto it = m_Materials.find(id);
    if(it == m_Materials.end()) 
    {
        printf("Could not find material with id %u. No update done.\n", id);
        return;
    }

    auto material = it->second;

    mats::MatsIO io = mats::MatsIO();

    mats::MaterialParameters params{};
    bool success = io.load((char*)data, size, &params);

    PBRUniforms materialData{};
    if(success){
        materialData.shaderUniforms.albedo = params.albedo;
        materialData.shaderUniforms.metallic = params.metallic;
        materialData.shaderUniforms.roughness = params.roughness;
        materialData.shaderUniforms.ambientOcclusions = params.ao;

        if(params.albedoTextureSize > 0){
            materialData.textures.albedo.filename = std::string(params.albedoTexture);
        }

        if(params.metallicTextureSize > 0){
            materialData.textures.metallic = std::string(params.metallicTexture);
        }

        if(params.roughnessTextureSize > 0){
            materialData.textures.roughness = std::string(params.roughnessTexture);
        }

        if(params.aoTextureSize > 0){
            materialData.textures.ambientOcclusion = std::string(params.aoTexture);
        }

        material->update(materialData, m_Device);
    }
}

void MaterialSystem::updateBindgroups()
{
    for(auto& material : m_Materials){
        material.second->updateBindGroup(m_Device);
    }
}

bool MaterialSystem::onUpdate(WGpuDevice* device, wgpu::Queue* queue)
{
    bool hasUpdates = false;
    for(auto& material : m_Materials){
        hasUpdates |= material.second->onUpdate(device, queue);
    }
    return hasUpdates;
}

void MaterialSystem::cleanup()
{
    m_Materials.clear();
}