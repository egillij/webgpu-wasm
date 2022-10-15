#pragma once

#include <string>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

#include "GameObject.h"

#include "Utils/UniformStructs.h"

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
    uint32_t id;
    std::string filename;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct MaterialDescription {
    uint32_t id;
    glm::vec3 color;
};

struct GameObjectNode {
    uint32_t id;
    
    int modelId;
    int materialId;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct SceneDescription {
    std::string name;

    // Information about what models are in the scene
    ModelDescription* modelDescriptions;
    uint64_t numberOfModels;

    MaterialDescription* materialDescriptons;
    uint64_t numberOfMaterials;

    GameObjectNode* gameObjects;
    uint64_t numberOfGameObjects;
};

class Scene {
public:
    Scene(const SceneDescription* description, WGpuDevice* device);
    ~Scene();

    void onUpdate();

private:
    WGpuBindGroup* getUniformsBindGroup();
    std::vector<GameObject*>& getGameObjects();

    friend class Renderer;

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
    
    SceneUniforms sceneUniforms;

    //Temporary
    
    

    WGpuDevice* device_;
};