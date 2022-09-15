#include "wgpuShader.h"

#include "wgpuDevice.h"

WGpuShader::WGpuShader(const std::string& name, const std::string& shaderCode, WGpuDevice* device)
: m_Name(name)
{
    wgpu::ShaderModuleWGSLDescriptor wgslDescription{};
    wgslDescription.source = shaderCode.c_str();

    wgpu::ShaderModuleDescriptor description{};
    description.label = m_Name.c_str();
    description.nextInChain = &wgslDescription;
    m_Module = device->getHandle().CreateShaderModule(&description);

    //TODO: make as part of input
    m_ColorTargets.reserve(1);

    wgpu::ColorTargetState cts{};
    cts.format = wgpu::TextureFormat::BGRA8Unorm;

    m_ColorTargets.emplace_back(cts);

    m_FragmentState.module = m_Module;
    m_FragmentState.entryPoint = "main_f";
    m_FragmentState.targetCount = m_ColorTargets.size();
    m_FragmentState.targets = m_ColorTargets.data();
}

WGpuShader::~WGpuShader()
{

}