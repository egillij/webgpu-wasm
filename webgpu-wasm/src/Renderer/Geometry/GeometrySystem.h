#pragma once

#include "TriangleMesh.h"

#include <unordered_map>
#include <memory>

class WGpuDevice;

class GeometrySystem final {
public:
    GeometrySystem(WGpuDevice* device);
    ~GeometrySystem();

    TriangleMesh* registerTriangleMesh(uint32_t id, const std::string& name, const std::string& filename);

    TriangleMesh* find(uint32_t id);

    void updateTriangleMesh(uint32_t id, void* data, int size);

    const TriangleMesh* getCube() const;

    void clear();

private:
    WGpuDevice* m_Device;
    std::unordered_map<uint32_t, std::shared_ptr<TriangleMesh>> m_TriangleMeshes;
    std::unique_ptr<TriangleMesh> m_UnitCube;
};