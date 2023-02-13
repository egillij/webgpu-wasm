// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include "Renderer/Pipelines/PathTracingPipeline.h"
#include "Renderer/Pipelines/PresentPipeline.h"

class WGpuDevice;
class SwapChain;

class PathTracer final {
public:
    PathTracer(uint32_t width, uint32_t height, WGpuDevice* device);
    ~PathTracer();

    void run();

    void render();

    void present();

private:
    WGpuDevice* m_Device;
    WGpuSwapChain* m_SwapChain;
    PathTracingPipeline* m_RenderPipeline;
    PresentPipeline* m_PresentPipeline;

};