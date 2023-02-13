// Copyright 2023 Egill Ingi Jacobsen

#include "LightmapProcessor.h"

class WGpuCubemap;
class WGpuTexture;
class WGpuDevice;
class Lightmap;

LightmapProcessor::LightmapProcessor(WGpuDevice* device)
: m_Device(device)
{

}

LightmapProcessor::~LightmapProcessor()
{

}

void LightmapProcessor::processHDR(WGpuTexture* texture, Lightmap* output)
{

}


void LightmapProcessor::generateDiffuse()
{

}

void LightmapProcessor::generateSpecular()
{

}
