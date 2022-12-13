#include "PathTracer.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"

#include "Application.h"
#include <emscripten/html5.h>

void frameReady(WGPUQueueWorkDoneStatus status, void* userData){
    if(status == WGPUQueueWorkDoneStatus_Success){
        if(!userData) printf("PathTracer  not attached to queue callback\n");
        PathTracer* pt = (PathTracer*)userData;
        pt->present();
    }
}

int ptAnimFrameRender(double t, void* userData) {
    Application::get()->onUpdate();
    return 1;
}

PathTracer::PathTracer(uint32_t width, uint32_t height, WGpuDevice* device)
: m_Device(device)
{
    //TODO: take in width and height
    m_SwapChain = new WGpuSwapChain(m_Device, width, height);
    m_RenderPipeline = new PathTracingPipeline(m_Device);
    m_PresentPipeline = new PresentPipeline(m_RenderPipeline->getTargetBuffer(), m_Device);
}

PathTracer::~PathTracer()
{
    if(m_SwapChain) delete m_SwapChain;
    m_SwapChain = nullptr;
    if(m_RenderPipeline) delete m_RenderPipeline;
    m_RenderPipeline = nullptr;
    if(m_PresentPipeline) delete m_PresentPipeline;
    m_PresentPipeline = nullptr;
}

void PathTracer::run()
{
    render();
    // present();
}

void PathTracer::render()
{
    wgpu::Queue queue = m_Device->getHandle().GetQueue();
    m_RenderPipeline->run(m_Device, &queue);

    queue.OnSubmittedWorkDone(0, frameReady, this);
}

void PathTracer::present()
{
    m_PresentPipeline->run(m_Device, m_SwapChain);

    emscripten_request_animation_frame(ptAnimFrameRender, this);
}