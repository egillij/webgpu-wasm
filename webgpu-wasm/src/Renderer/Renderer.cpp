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

#include "Scene/GameObject.h"
#include "Scene/Scene.h"

#include "Utils/UniformStructs.h"

#include <emscripten/emscripten.h>



Renderer::Renderer(uint32_t width, uint32_t height, WGpuDevice* device)
: m_Device(device)
{
    m_SwapChain = new WGpuSwapChain(m_Device, width, height);

    m_Pipeline = new PBRRenderPipeline(width, height, device);
    
}

Renderer::~Renderer()
{
    
}

void Renderer::render(Scene* scene)
{
    if(!scene) return; //TODO: report error?

    if(m_Pipeline) m_Pipeline->run(scene, m_Device, m_SwapChain);
    
    // printf("Triangles drawn: %i\n", frameTriangles);
}