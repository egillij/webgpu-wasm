#include "GameObject.h"

#include "Renderer/Geometry/TriangleMesh.h"
#include "Renderer/Geometry/Part.h"

#include "Renderer/Material.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"

#include "Renderer/MaterialSystem.h"
#include "Renderer/Geometry/GeometrySystem.h"

#include <emscripten.h>

GameObject::GameObject(const std::string& name)
: m_Name(name), m_BindGroup(nullptr), m_Material(nullptr)
{
    m_ModelUniforms.transform = glm::mat4(1.f);
    m_ModelUniforms.normalMatrix = glm::transpose(glm::inverse(m_ModelUniforms.transform));
}

GameObject::~GameObject()
{
    // for(int i = 0; i < m_Parts.size(); ++i) delete m_Parts[i].first;
    // m_Parts.clear();
    if(m_BindGroup) delete m_BindGroup;
}

void GameObject::setMesh(const uint32_t meshId, const uint32_t materialId, const glm::mat4& transform,
                         GeometrySystem* geometrySystem, MaterialSystem* materialSystem, WGpuDevice* device)
{
    // ModelData modelData = loadFromFile(meshFile, materialSystem);
    // for(size_t i = 0; i < modelData.modelData.size(); ++i){
    //     ModelData::PartData &part = modelData.modelData.at(i);

    //     TriangleMesh* mesh = new TriangleMesh(part.name);
    //     mesh->update(part.vertexData, part.numberOfVertices*8*sizeof(float), part.indexData, part.numberOfIndices, device);
    //     Part* part_ = new Part(part.name, mesh);
    //     m_Parts.push_back({part_, part.material});
    // }
    m_Mesh = geometrySystem->find(meshId);
    m_Material = materialSystem->find(materialId);

    m_ModelUniforms.transform = transform;

    m_UniformBuffer = new WGpuUniformBuffer(device, m_Name + "_UniformBuffer", sizeof(ModelUniforms));

    wgpu::Queue queue = device->getHandle().GetQueue();
    queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, (void*) &m_ModelUniforms, sizeof(ModelUniforms));
    
    m_BindGroupLayout = new WGpuBindGroupLayout(m_Name + "_BindGroupLayout");
    m_BindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(ModelUniforms), 0, wgpu::ShaderStage::Vertex);
    m_BindGroupLayout->build(device);

    m_BindGroup = new WGpuBindGroup(m_Name + "_BindGroup");
    m_BindGroup->setLayout(m_BindGroupLayout);
    m_BindGroup->addBuffer(m_UniformBuffer, BufferBindingType::Uniform, sizeof(ModelUniforms), 0, wgpu::ShaderStage::Vertex);
    m_BindGroup->build(device);
}

WGpuBindGroup* GameObject::getMaterialBindGroup()
{
    if(!m_Material) return nullptr;
    return m_Material->getBindGroup();
}