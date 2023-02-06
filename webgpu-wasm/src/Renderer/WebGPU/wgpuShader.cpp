#include "wgpuShader.h"

#include "wgpuDevice.h"

WGpuShader::WGpuShader(const std::string& name, const ShaderDescription& shaderDescription, WGpuDevice* device)
: m_Name(name)
{
    wgpu::ShaderModuleWGSLDescriptor wgslDescription{};
    wgslDescription.source = shaderDescription.shaderCode.c_str();

    wgpu::ShaderModuleDescriptor description{};
    description.label = m_Name.c_str();
    description.nextInChain = &wgslDescription;
    m_Module = device->getHandle().CreateShaderModule(&description);

    if(shaderDescription.type != ShaderType::COMPUTE){
        m_ColorTargets.reserve(shaderDescription.colorTargets.size());

        for(const TextureFormat& fmt : shaderDescription.colorTargets){
            wgpu::ColorTargetState cts{};
            cts.format = static_cast<wgpu::TextureFormat>(fmt);

            m_ColorTargets.emplace_back(cts);
        }
        

        m_FragmentState.module = m_Module;
        m_FragmentState.entryPoint = "main_f";
        m_FragmentState.targetCount = m_ColorTargets.size();
        m_FragmentState.targets = m_ColorTargets.data();
    }
}

WGpuShader::~WGpuShader()
{
    m_Module.Release();
}