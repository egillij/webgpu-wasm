#include "GameObject.h"

#include "Renderer/Geometry/TriangleMesh.h"
#include "Renderer/Geometry/Part.h"

#include "Renderer/Material.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"

#include "ModelLoader.h"
#include <emscripten.h>

ModelData loadFromFile(const std::string &filename)
{
    std::string m_ServerResource = "/resources/models/" + filename;
    std::string m_LocalResource = "./models/" + filename;
    emscripten_wget(m_ServerResource.c_str(), m_LocalResource.c_str());

    std::string m_ServerMtl = m_ServerResource.substr(0, m_ServerResource.size() - 3) + "mtl";
    std::string m_LocalMtl = m_LocalResource.substr(0, m_LocalResource.size() - 3) + "mtl";
    emscripten_wget(m_ServerMtl.c_str(), m_LocalMtl.c_str());

    ModelData model = ModelLoader::loadModelFromFile(m_LocalResource.c_str());
    return model;
}


GameObject::GameObject(const std::string& name)
:m_Name(name), m_BindGroup(nullptr), m_Material(nullptr)
{
    m_ModelUniforms.transform = glm::mat4(1.f);
    m_ModelUniforms.normalMatrix = glm::transpose(glm::inverse(m_ModelUniforms.transform));
}

GameObject::~GameObject()
{
    for(int i = 0; i < m_Parts.size(); ++i) delete m_Parts[i];
    m_Parts.clear();
    if(m_BindGroup) delete m_BindGroup;
}

void GameObject::setMesh(const std::string& meshFile, const glm::mat4& transform, WGpuDevice* device)
{
    ModelData modelData = loadFromFile(meshFile);
    for(size_t i = 0; i < modelData.modelData.size(); ++i){
        ModelData::PartData &part = modelData.modelData.at(i);

        TriangleMesh* mesh = new TriangleMesh(part.name);
        mesh->update(part.vertexData, part.numberOfVertices*8*sizeof(float), part.indexData, part.numberOfIndices, device);
        Part* part_ = new Part(part.name, mesh);
        m_Parts.push_back(part_);
    }

    
    m_ModelUniforms.transform = transform;

    m_UniformBuffer = new WGpuUniformBuffer(device, m_Name + "_UniformBuffer", sizeof(ModelUniforms));

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, (void*) &m_ModelUniforms, sizeof(ModelUniforms));
    
    m_BindGroupLayout = new WGpuBindGroupLayout(meshFile + "_BindGroupLayout");
    m_BindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(ModelUniforms), 0, wgpu::ShaderStage::Vertex);
    m_BindGroupLayout->build(device);

    m_BindGroup = new WGpuBindGroup(meshFile + "_BindGroup");
    m_BindGroup->setLayout(m_BindGroupLayout);
    m_BindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(ModelUniforms), 0, wgpu::ShaderStage::Vertex);
    m_BindGroup->build(device);

    // TODO: another function to register/add a material
    m_Material = new PBRMaterial("PBR test material", glm::vec3(0.2f, 0.4f, 0.8f), device);
}

WGpuBindGroup* GameObject::getMaterialBindGroup()
{
    if(!m_Material) return nullptr;
    return m_Material->getBindGroup();
}