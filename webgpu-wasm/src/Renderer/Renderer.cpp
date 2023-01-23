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
#include "Renderer/WebGPU/wgpuCubemap.h"

#include "Renderer/Pipelines/PBRRenderPipeline.h"
#include "Renderer/Pipelines/PresentPipeline.h"
#include "Renderer/Pipelines/CubemapVizualizationPipeline.h"

// Temporary
#include "Renderer/PreProcessors/CubemapGenerationPipeline.h"

#include "Scene/Environment.h"
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

    m_CubemapVizPipeline = new CubemapVizualizationPipeline(width, height, m_Device);

    m_DiffuseConvolutionPipeline = new CubemapGenerationPipeline(CubemapGenerationPipeline::PipelineType::EquirectangularToCubemap, m_Device);
    
}

Renderer::~Renderer()
{
    if(m_SwapChain) delete m_SwapChain;
    m_SwapChain = nullptr;
    if(m_Pipeline) delete m_Pipeline;
    m_Pipeline = nullptr;
    if(m_PresentPipeline) delete m_PresentPipeline;
    m_PresentPipeline = nullptr;
    if(m_Compute) delete m_Compute;
    m_Compute = nullptr;
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
    
    // if(m_Pipeline){
    //     m_Pipeline->run(scene, m_Device, m_SwapChain, &queue);
        
    //     //Wait for queue to finish
    //     // queue.OnSubmittedWorkDone(0, pbrDoneCallback, this);

    // }

    // if(m_Compute){
    //     m_Compute->run(m_Device, m_SwapChain);
    // } 

    
    //TODO: make and render a cubemap test
    static WGpuCubemap* cubemapTest = new WGpuCubemap("Test Cube Map", m_Device);
    // static TextureCreateInfo texInf{};
    // texInf.format = TextureFormat::RGBA32Float;
    // texInf.height = 1;
    // texInf.width = 1;
    // texInf.usage = {TextureUsage::TextureBinding, TextureUsage::CopyDst};
    // static WGpuTexture* temp = new WGpuTexture("Temp tex", &texInf);
    // static bool diffuseIrradiance = true;
    // if(diffuseIrradiance){
    //     m_DiffuseConvolutionPipeline->process(nullptr, cubemapTest);
    //     diffuseIrradiance = false;
    // }

    static bool updateMap = true;
    if(m_CubemapVizPipeline){
        if(updateMap){
            m_CubemapVizPipeline->setCubemap(scene->m_Environment->getBackground(), m_Device);
            updateMap = false;
        }
        m_CubemapVizPipeline->run(m_Device, m_SwapChain);
        emscripten_request_animation_frame(animFrameRender, nullptr);
    }
    

}

void Renderer::present()
{
    printf("Present\n");
    m_PresentPipeline->run(m_Device, m_SwapChain);
    emscripten_request_animation_frame(animFrameRender, nullptr);
}