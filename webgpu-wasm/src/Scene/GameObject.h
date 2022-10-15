#pragma once

#include <string>
#include <vector>

#include <glm/mat4x4.hpp>

#include "Utils/UniformStructs.h"

// class TriangleMesh;
class Part;
class WGpuDevice;
class WGpuBindGroup;
class WGpuBindGroupLayout;
class WGpuUniformBuffer;

class Material;

class GameObject {
public:
    GameObject(const std::string& name);
    ~GameObject();

    void setMesh(const std::string& meshFile, const glm::mat4& transform, WGpuDevice* device);
    // TriangleMesh* getMesh() { return m_Mesh; }
    std::vector<Part*>& getParts() { return m_Parts; }
    WGpuBindGroup* getModelBindGroup() { return m_BindGroup; }
    WGpuBindGroup* getMaterialBindGroup();

    const std::string& getName() const {return m_Name;}

private:
    std::string m_Name;

    ////////////
    // All of this is a model/part
    std::vector<Part*> m_Parts;
    // TriangleMesh* m_Mesh;
    ModelUniforms m_ModelUniforms;
    WGpuBindGroupLayout* m_BindGroupLayout;
    WGpuBindGroup* m_BindGroup;
    WGpuUniformBuffer* m_UniformBuffer;
    ////////////

    // Currently only one material per model, even if it is constructed of multiple parts. That should be changed.
    // Each renderable should have it's own material
    Material* m_Material;
};