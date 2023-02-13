// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <string>
#include <optional>

#include <glm/vec3.hpp>

class WGpuBindGroupLayout;
class WGpuBindGroup;
class WGpuDevice;
class WGpuTexture;
class WGpuSampler;
class WGpuUniformBuffer;

class ProceduralPipeline;

namespace wgpu {
    class Queue;
};

enum class MaterialType {
    PBR   
};

struct TextureOption {
    std::string filename = {};
    ProceduralPipeline* pipeline = nullptr;
    WGpuBindGroup* bindgroup = nullptr;
};

struct PBRUniforms {
    struct ShaderUniforms{
        glm::vec3 albedo = glm::vec3(0.9f, 0.2f, 0.3f);
        float metallic = 0.f;
        float roughness = 0.f;
        float ambientOcclusions = 0.1f;
        float padding[2];
    } shaderUniforms;

    struct Textures {
        TextureOption albedo{};

        std::string metallic = {};
        std::string roughness = {};
        std::string ambientOcclusion = {};
    } textures;
};

class Material {
public:
    // WGpuBindGroup* getBindGroup() { return m_MaterialBindGroup; }
    WGpuBindGroup* getBindGroup() const { return m_MaterialBindGroup; }
    ~Material();

    virtual void update(const PBRUniforms& data, WGpuDevice* device) = 0;
    virtual void updateBindGroup(WGpuDevice* device) = 0;

    virtual bool onUpdate(WGpuDevice* device, wgpu::Queue* queue) {return false;};

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
    PBRMaterial(const std::string& name, WGpuDevice* device);
    PBRMaterial(const std::string& name, const PBRUniforms& data, WGpuDevice* device);
    ~PBRMaterial();

    virtual void update(const PBRUniforms& data, WGpuDevice* device) override;
    virtual void updateBindGroup(WGpuDevice* device) override;

    virtual bool onUpdate(WGpuDevice* device, wgpu::Queue* queue) override;

private:
    PBRUniforms m_Uniforms;
    WGpuUniformBuffer* m_UniformBuffer;
    
    WGpuTexture* m_AlbedoTexture;
    WGpuTexture* m_MetallicTexture;
    WGpuTexture* m_RoughnessTexture;
    WGpuTexture* m_AoTexture;
};