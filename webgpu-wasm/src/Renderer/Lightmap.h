// Copyright 2023 Egill Ingi Jacobsen

#pragma once

class WGpuCubemap;

class Lightmap {
public:
    Lightmap();
    ~Lightmap();

    inline WGpuCubemap* getDiffuseIrradiance() { return m_Diffuse; }
    inline WGpuCubemap* getDiffuseIrradiance() { return m_Diffuse; }

private:
    WGpuCubemap* m_Diffuse;
    WGpuCubemap* m_Specular;

};