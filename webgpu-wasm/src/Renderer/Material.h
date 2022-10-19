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
    glm::vec3 albedo = glm::vec3(0.7f);
    float filler;
    glm::vec3 ambient = glm::vec3(0.1f);
    float filler2;
    glm::vec3 specular = glm::vec3(1.f);
    float shininess = 100.f;
};

class Material {
public:
    WGpuBindGroup* getBindGroup() { return m_MaterialBindGroup; }
    ~Material();

protected:
    Material(const std::string& name, MaterialType type);
    
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
    PBRMaterial(const std::string& name, const PBRUniforms& data, WGpuDevice* device);
    ~PBRMaterial();

private:
    PBRUniforms m_Uniforms;
    WGpuUniformBuffer* m_UniformBuffer;
    
    WGpuTexture* m_Texture;

    //TODO: samplers can be reused across textures. We should not store them here
    WGpuSampler* m_Sampler;
};