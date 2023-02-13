// Copyright 2023 Egill Ingi Jacobsen

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
    GameObject(const uint32_t id, const std::string& name);
    ~GameObject();

    void setTransform(const glm::mat4& transform);
    void setMesh(const uint32_t meshId, const uint32_t materialId,
                 GeometrySystem* geometrySystem, MaterialSystem* materialSystem, WGpuDevice* device);

    const uint32_t getId() const { return m_Id; }
    const glm::mat4& getTransform() { return m_Transform; }
    // TriangleMesh* getMesh() { return m_Mesh; }
    // std::vector<std::pair<Part*, Material*>>& getParts() { return m_Parts; }
    const TriangleMesh* getMesh() const { return m_Mesh; }
    const Material* getMaterial() const { return m_Material; }
    WGpuBindGroup* getModelBindGroup() { return m_BindGroup; }
    WGpuBindGroup* getMaterialBindGroup();

    const std::string& getName() const {return m_Name;}

    GameObject* getNext() { return next; }
    void setNext(GameObject* object) { next = object; }

    void cacheTransform(const glm::mat4& transform, WGpuDevice* device);

private:
    uint32_t m_Id;
    std::string m_Name;

    // Native transform
    glm::mat4 m_Transform;

    // Complete transforms used in shaders, i.e. hierarchically calculated transform
    ModelUniforms m_ModelUniforms;
    WGpuUniformBuffer* m_UniformBuffer;

    ////////////
    // All of this is a model/part
    TriangleMesh* m_Mesh;
    Material* m_Material;
    
    WGpuBindGroupLayout* m_BindGroupLayout;
    WGpuBindGroup* m_BindGroup;

    
    ////////////

    // TODO: ætti ég að hafa lista hér í staðinn?
    GameObject* next;
};