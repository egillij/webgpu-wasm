// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include "Material.h"

#include <unordered_map>
#include <memory>

class WGpuDevice;

namespace wgpu {
    class Queue;
};

class MaterialSystem {
public:
    MaterialSystem(WGpuDevice* device);
    ~MaterialSystem();

    Material* registerMaterial(uint32_t id, const std::string& name, const PBRUniforms& data);
    Material* registerMaterial(uint32_t id, const std::string& name, const std::string& filename);

    Material* find(uint32_t id);

    void updateMaterial(uint32_t id, void* data, int size);

    void updateBindgroups();

    bool onUpdate(WGpuDevice* device, wgpu::Queue* queue);

    void cleanup();

private:
    WGpuDevice* m_Device;
    std::unordered_map<uint32_t, std::shared_ptr<Material>> m_Materials;
};