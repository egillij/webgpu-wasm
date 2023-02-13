// Copyright 2023 Egill Ingi Jacobsen

#include "ComputePipeline.h"

#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuShader.h"

ComputePipeline::ComputePipeline(const std::string& label)
: m_Label(label), m_Pipeline(nullptr), m_Shader(nullptr)
{
}

ComputePipeline::~ComputePipeline()
{
    //TODO: cleanup all resource owned by pipeline
    if(m_Pipeline)
    {
        m_Pipeline.Release();
    }
}

void ComputePipeline::addBindGroup(WGpuBindGroupLayout* bindgroup)
{
    m_BindGroups.push_back(bindgroup);
}

void ComputePipeline::setShader(WGpuShader* shader)
{
    m_Shader = shader;
}

void ComputePipeline::build(WGpuDevice* device)
{
    std::vector<wgpu::BindGroupLayout> bindGroupLayouts;
    bindGroupLayouts.reserve(m_BindGroups.size());
    for(WGpuBindGroupLayout* bg : m_BindGroups){
        bindGroupLayouts.emplace_back(*bg->get());
    }
    
    wgpu::PipelineLayoutDescriptor plDescriptor{};
    std::string pldLabel = m_Label + "_computePipeline";
    plDescriptor.label = pldLabel.c_str();
    plDescriptor.bindGroupLayoutCount = bindGroupLayouts.size();
    plDescriptor.bindGroupLayouts = bindGroupLayouts.data();

    wgpu::PipelineLayout layout = device->getHandle().CreatePipelineLayout(&plDescriptor);

    wgpu::ProgrammableStageDescriptor psdesc{};
    psdesc.entryPoint = "main";
    psdesc.module = m_Shader->getModule();

    wgpu::ComputePipelineDescriptor desc{};
    desc.label = m_Label.c_str();
    desc.compute = psdesc;
    desc.layout = layout;

    m_Pipeline = device->getHandle().CreateComputePipeline(&desc);
}