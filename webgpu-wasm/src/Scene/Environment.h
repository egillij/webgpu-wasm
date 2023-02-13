// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <string>

class WGpuDevice;
class WGpuCubemap;

class Environment {
public:
    Environment(const std::string& name, const std::string& filename, WGpuDevice* device);
    ~Environment();

    inline WGpuCubemap* getBackground() { return m_Background; }
    inline WGpuCubemap* getDiffuseIrradiance() { return m_DiffuseIrradiance; }
    inline WGpuCubemap* getSpecularRadiance() { return m_SpecularRadiance; }

private:
    std::string m_Name;
    WGpuCubemap* m_Background;
    WGpuCubemap* m_DiffuseIrradiance;
    WGpuCubemap* m_SpecularRadiance;
};