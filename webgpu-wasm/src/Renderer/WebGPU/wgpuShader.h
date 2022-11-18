#pragma once

#include "wgpuTexture.h"

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

class WGpuDevice;

struct ShaderDescription {
    std::string shaderCode;
    std::vector<TextureFormat> colorTargets;
};

class WGpuShader {
public:
    WGpuShader() : m_Name("") {};
    WGpuShader(const std::string& name, const ShaderDescription& shaderDescription, WGpuDevice* device);
    ~WGpuShader();

    const wgpu::ShaderModule& getModule() const { return m_Module; }
    const wgpu::FragmentState* getFragmentState() const { return &m_FragmentState; }


private:
    std::string m_Name;
    wgpu::ShaderModule m_Module;

    std::vector<wgpu::ColorTargetState> m_ColorTargets;
    wgpu::FragmentState m_FragmentState;
};