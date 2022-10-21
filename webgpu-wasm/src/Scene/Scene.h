#pragma once

#include <string>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

#include "GameObject.h"

#include "Utils/UniformStructs.h"

class MaterialSystem;
class GeometrySystem;

class WGpuDevice;
class WGpuBindGroup;
class WGpuBindGroupLayout;
class WGpuUniformBuffer;

class WGpuSwapChain;
class WGpuPipeline;
class WGpuShader;
class WGpuTexture;
class WGpuSampler;

#define MATERIAL_NO_ID 0
#define MODEL_NO_ID 0
#define NODE_NO_ID 0

struct ModelDescription {
    uint32_t id = MODEL_NO_ID;
    std::string name = {};
    std::string filename = {};

    glm::vec3 position = glm::vec3(0.f);
    glm::vec3 rotation = glm::vec3(0.f);
    glm::vec3 scale = glm::vec3(1.f);
};

struct MaterialDescription {
    uint32_t id = MATERIAL_NO_ID;
    std::string name = {};
    std::string filename = {};
    glm::vec4 ambient = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
    glm::vec4 albedo = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
    glm::vec4 specular = glm::vec4(1.f);
    float shininess = 100.f;
};

struct GameObjectNode {
    uint32_t id = NODE_NO_ID;
    std::string name = {};

    int modelId = MODEL_NO_ID;
    int materialId = MATERIAL_NO_ID;

    glm::vec3 position = glm::vec3(0.f);
    glm::vec3 rotation = glm::vec3(0.f);
    glm::vec3 scale = glm::vec3(1.f);
};

struct SceneDescription {
    std::string name = {};

    // Information about what models are in the scene
    ModelDescription* modelDescriptions = nullptr;
    uint64_t numberOfModels = 0;

    MaterialDescription* materialDescriptons = nullptr;
    uint64_t numberOfMaterials = 0;

    GameObjectNode* gameObjects = nullptr;
    uint64_t numberOfGameObjects = 0;
};

class Scene {
public:
    Scene(const SceneDescription* description, MaterialSystem* materialSystem, GeometrySystem* geometrySystem, WGpuDevice* device);
    ~Scene();

    void onUpdate();

private:
    WGpuBindGroup* getUniformsBindGroup();
    std::vector<GameObject*>& getGameObjects();

    friend class Renderer;

    // Ekki skalanlegt að gera þetta svona. Breyta pipeline til að taka á móti
    // hlutum til að teikna. Nota material t.d. til að ákvarða í hvaða pipeline hlutur endar
    // Nota lista af unique material týpum til að búa til allar pipelines sem þarf?
    friend class PBRRenderPipeline;

private:
    std::string m_Name;
    std::vector<GameObject*> m_GameObjects;

    struct Camera {
        glm::vec3 position;
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