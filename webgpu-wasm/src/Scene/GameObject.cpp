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

GameObject::GameObject(const uint32_t id, const std::string& name)
: m_Id(id), m_Name(name), m_Transform(glm::mat4(1.f)), m_UniformBuffer(nullptr),
  m_BindGroup(nullptr), m_Material(nullptr), next(nullptr)
{
    cacheTransform(m_Transform, nullptr);
}

GameObject::~GameObject()
{
    if(m_BindGroup) delete m_BindGroup;
    if(m_UniformBuffer) delete m_UniformBuffer;
    if(m_BindGroupLayout) delete m_BindGroupLayout;
}

void GameObject::setTransform(const glm::mat4& transform)
{
    m_Transform = transform;
    cacheTransform(m_Transform, nullptr);
}

void GameObject::setMesh(const uint32_t meshId, const uint32_t materialId,
                         GeometrySystem* geometrySystem, MaterialSystem* materialSystem, WGpuDevice* device)
{
    m_Mesh = geometrySystem->find(meshId);
    m_Material = materialSystem->find(materialId);

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

void GameObject::cacheTransform(const glm::mat4& transform, WGpuDevice* device)
{
    m_ModelUniforms.transform = transform;
    m_ModelUniforms.normalMatrix = glm::transpose(glm::inverse(m_ModelUniforms.transform));

    if(device && m_UniformBuffer){
        wgpu::Queue queue = device->getHandle().GetQueue();
        queue.WriteBuffer(m_UniformBuffer->getHandle(), 0, (void*) &m_ModelUniforms, sizeof(ModelUniforms));
    }
}