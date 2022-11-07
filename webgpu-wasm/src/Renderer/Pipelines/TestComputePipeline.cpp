#include "TestComputePipeline.h"

#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"

#include "Application.h"

#include <emscripten/html5.h>

static const char computeCode[] = R"(

fn noise(x : u32, y : u32) -> f32 {
    var n : u32 = x + y * 57;
    
    n = (n << 13) ^ n;

    let temp : u32 = (n * ((n * n * 15731) + 789221) +  1376312589) & 0x7fffffff; 
    let n_f : f32 = f32(temp); 
    return (1.0 - n_f / 1073741824.0);
}

@group(0) @binding(0) var testTexture : texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(1, 1, 1)
fn main() {
    

  for(var x : u32 = 0; x < 500; x++) {
    for(var y : u32 = 0; y < 500; y++) {
        // let u : f32 = f32(x)/500.0;
        // let v : f32 = f32(y)/500.0;
        let uv : vec2<u32> = vec2<u32>(x,y);
        let n : f32 = noise(x, y);
        let color : vec4<f32> = vec4<f32>(n, n, n, 1.0);
        textureStore(testTexture, uv, color);
    }
  }
}
)";

int computeAnimationFrame(double t, void* userData) {
    // params.pipeline->run(params.scene, params.device, params.swapChain);
    Application::get()->onUpdate();
    return 1;
}

void computeDoneCallback(WGPUQueueWorkDoneStatus status, void * userdata)
{
    if(status == WGPUQueueWorkDoneStatus::WGPUQueueWorkDoneStatus_Success){
        // printf("Compute pipeline success\n");
        TestComputePipeline* p = (TestComputePipeline*)userdata;
        p->present();
        emscripten_request_animation_frame(computeAnimationFrame, nullptr);
    }
    else {
        printf("Compute pipeline failure\n");
    }
}

TestComputePipeline::TestComputePipeline(WGpuDevice* device)
{
    m_Device = device;
    TextureCreateInfo info{};
    info.format = TextureFormat::RGBA8Unorm;
    info.width = 500;
    info.height = 500;
    info.usage = {TextureUsage::StorageBinding, TextureUsage::TextureBinding, TextureUsage::CopyDst};
    m_Texture = new WGpuTexture("Test Compute Texture", &info, device);

    wgpu::ShaderModuleWGSLDescriptor wgslDescription{};
    wgslDescription.source = computeCode;

    wgpu::ShaderModuleDescriptor shaderDesc{};
    shaderDesc.label = "Test compute shader";
    shaderDesc.nextInChain = &wgslDescription;

    m_Module = device->getHandle().CreateShaderModule(&shaderDesc);

    wgpu::BindGroupLayoutEntry bglentry{};
    bglentry.binding = 0;
    bglentry.visibility = wgpu::ShaderStage::Compute;

    wgpu::StorageTextureBindingLayout storageTexLayout{};
    storageTexLayout.access = wgpu::StorageTextureAccess::WriteOnly;
    storageTexLayout.format = wgpu::TextureFormat::RGBA8Unorm;
    storageTexLayout.viewDimension = wgpu::TextureViewDimension::e2D;

    bglentry.storageTexture = storageTexLayout;


    wgpu::BindGroupLayoutDescriptor bgld{};
    bgld.entryCount = 1;
    bgld.entries = &bglentry;

    wgpu::BindGroupLayout bgl = device->getHandle().CreateBindGroupLayout(&bgld);

    wgpu::BindGroupEntry entry{};
    entry.binding = 0;
    entry.textureView = m_Texture->createView();
    

    wgpu::BindGroupDescriptor bgd{};
    bgd.entryCount = 1;
    bgd.entries = &entry;
    bgd.layout = bgl;
    m_BindGroup = device->getHandle().CreateBindGroup(&bgd);
    

    wgpu::PipelineLayoutDescriptor layoutdesc{};
    layoutdesc.bindGroupLayoutCount = 1;
    layoutdesc.bindGroupLayouts = &bgl;
    

    wgpu::PipelineLayout layout = device->getHandle().CreatePipelineLayout(&layoutdesc);

    wgpu::ProgrammableStageDescriptor psdesc{};
    psdesc.entryPoint = "main";
    psdesc.module = m_Module;

    wgpu::ComputePipelineDescriptor desc{};
    desc.label = "Test Compute Pipeline";
    desc.compute = psdesc;
    desc.layout = layout;

    m_Pipeline = device->getHandle().CreateComputePipeline(&desc);
    
    m_PresentPipeline = new PresentPipeline(m_Texture, device);
}

TestComputePipeline::~TestComputePipeline()
{

}

void TestComputePipeline::run(WGpuDevice* device, WGpuSwapChain* swapChain)
{
    m_SwapChain = swapChain;
    wgpu::Queue queue = device->getHandle().GetQueue();

    wgpu::ComputePassDescriptor desc{};
    desc.label = "Test Compute pass descriptor";

    wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();

    wgpu::ComputePassEncoder computePass = encoder.BeginComputePass(&desc);
    computePass.SetPipeline(m_Pipeline);
    computePass.SetLabel("Test Compute Pass");
    computePass.SetBindGroup(0, m_BindGroup);

    computePass.DispatchWorkgroups(1, 1, 1);

    computePass.End();

    wgpu::CommandBuffer commands = encoder.Finish();

    queue.Submit(1, &commands);

    queue.OnSubmittedWorkDone(0, computeDoneCallback, this);

}

void TestComputePipeline::present()
{
    m_PresentPipeline->run(m_Device, m_SwapChain);
}