#pragma once

#include <string>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

#include "TriangleMesh.h"
#include "GameObject.h"

class WGpuDevice;
class WGpuBindGroup;
class WGpuBindGroupLayout;
class WGpuUniformBuffer;

class WGpuSwapChain;
class WGpuPipeline;
class WGpuShader;
class WGpuTexture;
class WGpuSampler;

struct ModelDescription {
    std::string filename;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    //TODO: Material
};

struct SceneDescription {
    std::string name;

    // Information about what models are in the scene
    ModelDescription* modelDescriptions;
    uint64_t numberOfModels;
};

class Scene {
public:
    Scene(const SceneDescription* description, WGpuDevice* device);
    ~Scene();

    void onUpdate();

private:
    std::string m_Name;
    std::vector<GameObject*> m_GameObjects;

    struct Camera {
        glm::mat4x4 projectionMatrix;
        glm::mat4x4 viewMatrix;
    } m_Camera;

    WGpuUniformBuffer* sceneUniformBuffer = nullptr;
    WGpuBindGroupLayout* sceneUniformBindGroupLayout = nullptr;
    WGpuBindGroup* sceneUniformBindGroup = nullptr;
    struct SceneUniforms {
        glm::mat4 viewProjection;
    };
    SceneUniforms sceneUniforms;

    //Temporary
    WGpuSwapChain* wSwapChain = nullptr;

    WGpuShader* shader = nullptr;
    
    WGpuPipeline* pipeline = nullptr;
    WGpuBindGroupLayout* materialBindGroupLayout = nullptr;
    WGpuBindGroup* materialBindGroup = nullptr;
    WGpuBindGroupLayout* modelUniformBindGroupLayout = nullptr;
    WGpuTexture* texture = nullptr;
    WGpuSampler* sampler = nullptr;
    WGpuTexture* depthTexture = nullptr;

    WGpuDevice* device_;
};