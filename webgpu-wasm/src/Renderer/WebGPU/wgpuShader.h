#pragma once

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuDevice;

class WGpuShader {
public:
    WGpuShader() : m_Name("") {};
    WGpuShader(const std::string& name, const std::string& shaderCode, WGpuDevice* device);
    ~WGpuShader();

    const wgpu::ShaderModule& getModule() const { return m_Module; }
    const wgpu::FragmentState* getFragmentState() const { return &m_FragmentState; }


private:
    std::string m_Name;
    wgpu::ShaderModule m_Module;

    std::vector<wgpu::ColorTargetState> m_ColorTargets;
    wgpu::FragmentState m_FragmentState;
};