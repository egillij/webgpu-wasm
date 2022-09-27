#include "wgpuPipeline.h"

#include "wgpuDevice.h"
#include "wgpuBindGroup.h"
#include "wgpuShader.h"

WGpuPipeline::WGpuPipeline()
{
}

WGpuPipeline::~WGpuPipeline()
{
}

void WGpuPipeline::addBindGroup(WGpuBindGroupLayout* bindgroup)
{
    m_BindGroups.push_back(bindgroup);
}

void WGpuPipeline::setShader(WGpuShader* shader)
{
    m_Shader = shader;
}

void WGpuPipeline::build(WGpuDevice* device)
{
    std::vector<wgpu::BindGroupLayout> bindGroupLayouts;
    bindGroupLayouts.reserve(m_BindGroups.size());
    for(WGpuBindGroupLayout* bg : m_BindGroups){
        bindGroupLayouts.emplace_back(*bg->get());
    }

    wgpu::PipelineLayoutDescriptor plDescriptor{};
    plDescriptor.bindGroupLayoutCount = m_BindGroups.size();
    plDescriptor.bindGroupLayouts = bindGroupLayouts.data();

    ////////////////////////
    // Make a class/struct for this creation that can be shared between the vertexbuffer and pipelines

    wgpu::VertexAttribute vAttribute[3];

    vAttribute[0].format = wgpu::VertexFormat::Float32x3;
    vAttribute[0].offset = 0;
    vAttribute[0].shaderLocation = 0;

    vAttribute[1].format = wgpu::VertexFormat::Float32x3;
    vAttribute[1].offset = 3*sizeof(float);
    vAttribute[1].shaderLocation = 1;

    vAttribute[2].format = wgpu::VertexFormat::Float32x2;
    vAttribute[2].offset = 6*sizeof(float);
    vAttribute[2].shaderLocation = 2;
    
    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.attributeCount = 3;
    vertexBufferLayout.attributes = vAttribute;
    vertexBufferLayout.arrayStride = 8 * sizeof(float);
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
    ////////////////////////

    wgpu::RenderPipelineDescriptor rpDescription{};
    rpDescription.layout = device->getHandle().CreatePipelineLayout(&plDescriptor);

    rpDescription.vertex.module = m_Shader->getModule();
    rpDescription.vertex.entryPoint = "main_v";
    rpDescription.vertex.buffers = &vertexBufferLayout;
    rpDescription.vertex.bufferCount = 1;

    rpDescription.fragment = m_Shader->getFragmentState();
    rpDescription.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    rpDescription.primitive.frontFace = wgpu::FrontFace::CCW;
    rpDescription.primitive.cullMode = wgpu::CullMode::Back;

    wgpu::DepthStencilState depthStencil{};
    depthStencil.format = wgpu::TextureFormat::Depth32Float;
    depthStencil.depthWriteEnabled = true;
    depthStencil.depthCompare = wgpu::CompareFunction::LessEqual;

    rpDescription.depthStencil = &depthStencil;
    
    m_Pipeline = device->getHandle().CreateRenderPipeline(&rpDescription);

}
