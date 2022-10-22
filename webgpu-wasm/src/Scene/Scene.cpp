#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"

#include <webgpu/webgpu_cpp.h>
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuPipeline.h"

#include "Renderer/MaterialSystem.h"
#include "Renderer/Geometry/GeometrySystem.h"

#include <emscripten.h>



///////////////////////////////////////////////////////////////////
// Float3 position, Float3 normal, Float2 uv
static float cubeVertices[6*6*8] = {
    1.f, -1.f, 1.f,   0.f, -1.f, 0.f,    1.f, 1.f,
  -1.f, -1.f, 1.f,    0.f, -1.f, 0.f,    0.f, 1.f,
  -1.f, -1.f, -1.f,   0.f, -1.f, 0.f,    0.f, 0.f,
  1.f, -1.f, -1.f,    0.f, -1.f, 0.f,    1.f, 0.f,
  1.f, -1.f, 1.f,     0.f, -1.f, 0.f,    1.f, 1.f,
  -1.f, -1.f, -1.f,   0.f, -1.f, 0.f,    0.f, 0.f,

  1.f, 1.f, 1.f,      1.f, 0.f, 0.f,    1.f, 1.f,
  1.f, -1.f, 1.f,     1.f, 0.f, 0.f,    0.f, 1.f,
  1.f, -1.f, -1.f,    1.f, 0.f, 0.f,    0.f, 0.f,
  1.f, 1.f, -1.f,     1.f, 0.f, 0.f,    1.f, 0.f,
  1.f, 1.f, 1.f,      1.f, 0.f, 0.f,    1.f, 1.f,
  1.f, -1.f, -1.f,    1.f, 0.f, 0.f,    0.f, 0.f,

  -1.f, 1.f, 1.f,     0.f, 1.f, 0.f,    1.f, 1.f,
  1.f, 1.f, 1.f,      0.f, 1.f, 0.f,    0.f, 1.f,
  1.f, 1.f, -1.f,     0.f, 1.f, 0.f,    0.f, 0.f,
  -1.f, 1.f, -1.f,    0.f, 1.f, 0.f,    1.f, 0.f,
  -1.f, 1.f, 1.f,     0.f, 1.f, 0.f,    1.f, 1.f,
  1.f, 1.f, -1.f,     0.f, 1.f, 0.f,    0.f, 0.f,

  -1.f, -1.f, 1.f,    -1.f, 0.f, 0.f,    1.f, 1.f,
  -1.f, 1.f, 1.f,     -1.f, 0.f, 0.f,    0.f, 1.f,
  -1.f, 1.f, -1.f,    -1.f, 0.f, 0.f,    0.f, 0.f,
  -1.f, -1.f, -1.f,   -1.f, 0.f, 0.f,    1.f, 0.f,
  -1.f, -1.f, 1.f,    -1.f, 0.f, 0.f,    1.f, 1.f,
  -1.f, 1.f, -1.f,    -1.f, 0.f, 0.f,    0.f, 0.f,

  1.f, 1.f, 1.f,      0.f, 0.f, 1.f,    1.f, 1.f,
  -1.f, 1.f, 1.f,     0.f, 0.f, 1.f,    0.f, 1.f,
  -1.f, -1.f, 1.f,    0.f, 0.f, 1.f,    0.f, 0.f,
  -1.f, -1.f, 1.f,    0.f, 0.f, 1.f,    0.f, 0.f,
  1.f, -1.f, 1.f,     0.f, 0.f, 1.f,    1.f, 0.f,
  1.f, 1.f, 1.f,      0.f, 0.f, 1.f,    1.f, 1.f,

  1.f, -1.f, -1.f,    0.f, 0.f, -1.f,    1.f, 1.f,
  -1.f, -1.f, -1.f,   0.f, 0.f, -1.f,    0.f, 1.f,
  -1.f, 1.f, -1.f,    0.f, 0.f, -1.f,    0.f, 0.f,
  1.f, 1.f, -1.f,     0.f, 0.f, -1.f,    1.f, 0.f,
  1.f, -1.f, -1.f,    0.f, 0.f, -1.f,    1.f, 1.f,
  -1.f, 1.f, -1.f,    0.f, 0.f, -1.f,    0.f, 0.f,
};

static uint64_t numCubeIndices = 6*6;
static uint32_t cubeIndices[6*6] = {
    0, 1, 2,
    3, 4, 5,

    6, 7, 8,
    9, 10, 11,

    12, 13, 14,
    15, 16, 17,

    18, 19, 20,
    21, 22, 23,

    24, 25, 26,
    27, 28, 29,

    30, 31, 32,
    33, 34, 35
};



/////////////////////////////////////////////////////////////////////////

static uint32_t WINDOW_WIDTH = 800;
static uint32_t WINDOW_HEIGHT = 600;
static float aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
static float fovY = glm::radians(45.f);

Scene::Scene(const SceneDescription* description, MaterialSystem* materialSystem, GeometrySystem* geometrySystem, WGpuDevice* device)
{
    m_Name = description->name;

    // Temporary???? Þarf kannski ekki að hafa hér
    device_ = device;

    m_Camera.projectionMatrix = glm::perspective(fovY, aspect, 0.01f, 100.f);
    m_Camera.viewMatrix = glm::lookAt(glm::vec3(-5.f, 2.f, 5.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

    sceneUniformBuffer = new WGpuUniformBuffer(device, "Scene Uniform Buffer", sizeof(SceneUniforms));

    sceneUniformBindGroupLayout = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    sceneUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sceneUniformBuffer->getSize(), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    sceneUniformBindGroupLayout->build(device_);

    sceneUniformBindGroup = new WGpuBindGroup("Scene Uniform Bind Group");
    sceneUniformBindGroup->setLayout(sceneUniformBindGroupLayout);
    sceneUniformBindGroup->addBuffer(sceneUniformBuffer, BufferBindingType::Uniform, sceneUniformBuffer->getSize(), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    // sceneUniformBindGroup->addSampler(sampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    // sceneUniformBindGroup->addTexture(texture, TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    sceneUniformBindGroup->build(device_);

    SamplerCreateInfo samplerInfo{};
    m_NearestSampler = new WGpuSampler("Sampler", &samplerInfo, device_);

    m_SamplerBindGroupLayout = new WGpuBindGroupLayout("Sampler Bind Group Layout");
    m_SamplerBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    m_SamplerBindGroupLayout->build(device);

    m_SamplerBindGroup = new WGpuBindGroup("Sampler Bind Group");
    m_SamplerBindGroup->setLayout(m_SamplerBindGroupLayout);
    m_SamplerBindGroup->addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    m_SamplerBindGroup->build(device);

    // Make scene from description

    for(int i = 0; i < description->numberOfModels; ++i){
        ModelDescription* model = description->modelDescriptions+i;
        std::string m_ServerResource = "/resources/models/" + model->filename;
        std::string m_LocalResource = "./models/" + model->filename;
        //TODO: make async
        emscripten_wget(m_ServerResource.c_str(), m_LocalResource.c_str());
        geometrySystem->registerTriangleMesh(model->id, model->name, m_LocalResource);
    }

    for(int i = 0; i < description->numberOfMaterials; ++i){
        MaterialDescription* material = description->materialDescriptons+i;


        PBRUniforms mat{};
        if(material->filename.empty()){
            mat.shaderUniforms.albedo = material->albedo;
            mat.shaderUniforms.specular = material->specular;
            mat.shaderUniforms.ambient = material->ambient;
            mat.shaderUniforms.shininess = material->shininess;

            materialSystem->registerMaterial(material->id, material->name, mat);
        }
        else {
            std::string m_ServerResource = "/resources/models/" + material->filename;
            std::string m_LocalResource = "./models/" + material->filename;
            //TODO: gera async og ekki hér, heldur sem hluti af register material???
            emscripten_wget(m_ServerResource.c_str(), m_LocalResource.c_str());

            materialSystem->registerMaterial(material->id, material->name, m_LocalResource);
        }
        
    }

    for(int i = 0; i < description->numberOfGameObjects; ++i){
        //TODO: Make and register meshes here instead of gameobjects
        GameObjectNode* node = description->gameObjects + i;
        GameObject* object = new GameObject(node->name);
        glm::mat4 rotation = glm::mat4(1.f);
        rotation = glm::rotate(glm::mat4(1.f), node->rotation.z, glm::vec3(0.f, 0.f, 1.f));
        rotation = rotation * glm::rotate(glm::mat4(1.f), node->rotation.y, glm::vec3(0.f, 1.f, 0.f));
        rotation = rotation * glm::rotate(glm::mat4(1.f), node->rotation.x, glm::vec3(1.f, 0.f, 0.f));

        glm::mat4 transform = rotation * glm::translate(glm::mat4(1.f), node->position) * glm::scale(glm::mat4(1.f), node->scale);
        object->setMesh(node->modelId, node->materialId, transform, geometrySystem, materialSystem, device);

        m_GameObjects.push_back(object);
    }

    //TODO: Make and register materials

    //TODO: make gameobjects and construct the scene hierarchy
}

Scene::~Scene()
{
    for(int i = 0; i < m_GameObjects.size(); ++i){
        delete m_GameObjects[i];
    }
    m_GameObjects.clear();
}

void Scene::onUpdate()
{
    wgpu::Queue queue = device_->getHandle().GetQueue();
    
    static float t = 0.0f;
    // float weight = glm::abs(glm::sin(t*glm::pi<float>()/10.f));
    // uniformColor.color = glm::vec4(1.f, 0.502f, 0.f, 1.f) * weight + glm::vec4(0.f, 0.498f, 1.f, 1.f) * (1.f-weight);
    
    static float radius = 3.f;
    static float phi = 0.f;
    static float theta = 0.1f;

    static glm::vec3 focusPoint = glm::vec3(0.f, 1.5f, 0.f);

    float x = radius * glm::cos(phi) * glm::sin(theta);
    float y = radius * glm::sin(phi) * glm::sin(theta);
    float z = radius * glm::cos(theta);

    //TODO: make camera spin
    static glm::vec3 cameraPosition;
    m_Camera.position = glm::vec3(x, y, z) + focusPoint;
    m_Camera.viewMatrix = glm::lookAt(m_Camera.position, focusPoint, glm::vec3(0.f, 1.f, 0.f));
    sceneUniforms.viewProjection = m_Camera.projectionMatrix * m_Camera.viewMatrix;
    sceneUniforms.cameraPosition = m_Camera.position;


    // printf("Frame %f\n", weight);

    t += 1.f/60.f;
    // phi = glm::clamp(glm::radians(10.f)*t, 0.f, glm::radians(180.f));
    theta = glm::radians(5.f)*t;
    // radius = 50.f*glm::cos(t*glm::radians(5.f));

    queue.WriteBuffer(sceneUniformBuffer->getHandle(), 0, (void*) &sceneUniforms, sizeof(SceneUniforms));

}

WGpuBindGroup* Scene::getUniformsBindGroup()
{
    return sceneUniformBindGroup;
}

std::vector<GameObject*>& Scene::getGameObjects()
{
    return m_GameObjects;
}