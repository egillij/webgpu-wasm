#pragma once

#include <string>
#include <vector>

#include <glm/mat4x4.hpp>

#include "Utils/UniformStructs.h"

class WGpuDevice;
class WGpuBindGroup;
class WGpuBindGroupLayout;
class WGpuUniformBuffer;

class TriangleMesh;
class Material;
class MaterialSystem;
class GeometrySystem;

class GameObject {
public:
    GameObject(const std::string& name);
    ~GameObject();

    void setMesh(const uint32_t meshId, const uint32_t materialId, const glm::mat4& transform,
                 GeometrySystem* geometrySystem, MaterialSystem* materialSystem, WGpuDevice* device);
    // TriangleMesh* getMesh() { return m_Mesh; }
    // std::vector<std::pair<Part*, Material*>>& getParts() { return m_Parts; }
    const TriangleMesh* getMesh() const { return m_Mesh; }
    const Material* getMaterial() const { return m_Material; }
    WGpuBindGroup* getModelBindGroup() { return m_BindGroup; }
    WGpuBindGroup* getMaterialBindGroup();

    const std::string& getName() const {return m_Name;}

private:
    std::string m_Name;

    ////////////
    // All of this is a model/part
    TriangleMesh* m_Mesh;
    Material* m_Material;
    
    ModelUniforms m_ModelUniforms;
    WGpuBindGroupLayout* m_BindGroupLayout;
    WGpuBindGroup* m_BindGroup;
    WGpuUniformBuffer* m_UniformBuffer;
    ////////////
};