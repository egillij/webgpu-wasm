// Copyright 2023 Egill Ingi Jacobsen

#pragma once

class WGpuCubemap;
class WGpuTexture;
class WGpuDevice;
class Lightmap;

class LightmapProcessor {
public:
    LightmapProcessor(WGpuDevice* device);
    ~LightmapProcessor();

    void processHDR(WGpuTexture* texture, Lightmap* output);

private:
    void generateDiffuse();
    void generateSpecular();

private:
    WGpuDevice* m_Device;

};