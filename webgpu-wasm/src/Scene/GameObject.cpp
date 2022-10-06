#include "GameObject.h"

#include "Renderer/TriangleMesh.h"

#include "Renderer/Material.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"

GameObject::GameObject(const std::string& name)
:m_Name(name), m_Mesh(nullptr), m_BindGroup(nullptr), m_Material(nullptr)
{
    m_ModelUniforms.transform = glm::mat4(1.f);
}

GameObject::~GameObject()
{
    if(m_Mesh) delete m_Mesh;
    if(m_BindGroup) delete m_BindGroup;
}

void GameObject::setMesh(const std::string& meshFile, const glm::mat4& transform, WGpuDevice* device)
{
    m_Mesh = new TriangleMesh(m_Name);
    m_Mesh->loadFromFile(meshFile, device);
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