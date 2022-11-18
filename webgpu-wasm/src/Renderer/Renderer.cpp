#include "Renderer.h"

#include "Renderer/Geometry/TriangleMesh.h"
#include "Renderer/Geometry/Part.h"
#include "Renderer/Material.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"

#include "Renderer/Pipelines/PBRRenderPipeline.h"
#include "Renderer/Pipelines/PresentPipeline.h"

#include "Scene/GameObject.h"
#include "Scene/Scene.h"

#include "Application.h"

#include "Utils/UniformStructs.h"

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>



Renderer::Renderer(uint32_t width, uint32_t height, WGpuDevice* device)
: m_Device(device)
{
    m_SwapChain = new WGpuSwapChain(m_Device, width, height);

    m_Pipeline = new PBRRenderPipeline(width, height, device);

    m_PresentPipeline = new PresentPipeline(m_Pipeline->getOutputTexture(), device);

    m_Compute = new TestComputePipeline(m_Device);
    
}

Renderer::~Renderer()
{
    
}

void pbrDoneCallback(WGPUQueueWorkDoneStatus status, void* userData){
    if(status == WGPUQueueWorkDoneStatus_Success){
        if(!userData) printf("Renderer  not attached to queue callback\n");
        Renderer* renderer = (Renderer*)userData;
        renderer->present();
    }
}

int animFrameRender(double t, void* userData) {
    // params.pipeline->run(params.scene, params.device, params.swapChain);
    Application::get()->onUpdate();
    return 1;
}

void Renderer::render(Scene* scene)
{
    if(!scene) return; //TODO: report error?

    wgpu::Queue queue = m_Device->getHandle().GetQueue();

    if(m_Pipeline){
        m_Pipeline->run(scene, m_Device, m_SwapChain, &queue);
        
        //Wait for queue to finish
        // queue.OnSubmittedWorkDone(0, pbrDoneCallback, this);

    } 
    // if(m_Compute){
    //     m_Compute->run(m_Device, m_SwapChain);
    // }

    
    // printf("Triangles drawn: %i\n", frameTriangles);
}

void Renderer::present()
{
    printf("Present\n");
    m_PresentPipeline->run(m_Device, m_SwapChain);
    emscripten_request_animation_frame(animFrameRender, nullptr);
}