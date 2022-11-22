#include "PathTracer.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"

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

}

void PathTracer::run()
{
    render();
    present();
}

void PathTracer::render()
{
    m_RenderPipeline->run(m_Device);
}

void PathTracer::present()
{
    m_PresentPipeline->run(m_Device, m_SwapChain);
}