#pragma once

#include "Material.h"

#include <unordered_map>
#include <memory>

class WGpuDevice;

class MaterialSystem {
public:
    MaterialSystem(WGpuDevice* device);
    ~MaterialSystem();

    Material* registerMaterial(uint32_t id, const std::string& name, const PBRUniforms& data);
    Material* registerMaterial(uint32_t id, const std::string& name, const std::string& filename);

    Material* find(uint32_t id);

private:
    WGpuDevice* m_Device;
    std::unordered_map<uint32_t, std::shared_ptr<Material>> m_Materials;
};