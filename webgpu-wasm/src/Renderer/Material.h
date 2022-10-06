#pragma once

#include <string>

#include <glm/vec3.hpp>

class WGpuBindGroupLayout;
class WGpuBindGroup;
class WGpuDevice;
class WGpuTexture;
class WGpuSampler;
class WGpuUniformBuffer;

enum class MaterialType {
    PBR   
};

struct PBRUniforms {
    glm::vec3 m_Color;
};

class Material {
public:
    WGpuBindGroup* getBindGroup() { return m_MaterialBindGroup; }

protected:
    Material(const std::string& name, MaterialType type);
    ~Material();
// private:
    std::string m_Name;
    MaterialType m_Type;

    WGpuBindGroupLayout* m_MaterialBindGroupLayout;
    WGpuBindGroup* m_MaterialBindGroup;
    WGpuUniformBuffer* m_UniformBuffer;
};

class PBRMaterial final : public Material {
public:
    PBRMaterial(const std::string& name, const glm::vec3& color, WGpuDevice* device);
    ~PBRMaterial();

private:
    PBRUniforms m_Uniforms;
    WGpuUniformBuffer* m_UniformBuffer;
    
    WGpuTexture* m_Texture;

    //TODO: samplers can be reused across textures. We should not store them here
    WGpuSampler* m_Sampler;
};