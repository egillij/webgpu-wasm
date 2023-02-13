    // Copyright 2023 Egill Ingi Jacobsen
    
    #include "NoisePipeline.h"
    
    #include "Renderer/WebGPU/wgpuDevice.h"
    #include "Renderer/WebGPU/wgpuTexture.h"
    #include "Renderer/WebGPU/wgpuShader.h"
    #include "Renderer/WebGPU/wgpuBindGroup.h"
    #include "Renderer/WebGPU/ComputePipeline.h"

    #include <webgpu/webgpu_cpp.h>

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

    NoisePipeline::NoisePipeline(WGpuDevice* device)
    : ProceduralPipeline("Noise Pipeline")
    {
        ShaderDescription shaderDesc{};
        shaderDesc.type = ShaderType::COMPUTE;
        shaderDesc.shaderCode = computeCode;
        m_Shader = new WGpuShader("Test Compute Shader", shaderDesc, device);

        m_BindGroupLayout = new WGpuBindGroupLayout("Compute Test BGL");

        m_BindGroupLayout->addStorageTexture(wgpu::StorageTextureAccess::WriteOnly, TextureFormat::RGBA8Unorm, wgpu::TextureViewDimension::e2D, 0, wgpu::ShaderStage::Compute);
        m_BindGroupLayout->build(device);
        
        m_Pipeline = new ComputePipeline("Test Compute"); //device->getHandle().CreateComputePipeline(&desc);
        m_Pipeline->setShader(m_Shader);
        m_Pipeline->addBindGroup(m_BindGroupLayout);
        m_Pipeline->build(device);
    }

    NoisePipeline::~NoisePipeline()
    {

    }

    void NoisePipeline::run(WGpuBindGroup* textureBindGroup, WGpuDevice* device, wgpu::Queue* queue)
    {
        wgpu::ComputePassDescriptor desc{};
        desc.label = "Noise Pipeline compute descriptor";

        wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();

        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass(&desc);
        computePass.SetPipeline(m_Pipeline->getPipeline());
        computePass.SetLabel("Noise Pipeline");
        computePass.SetBindGroup(0, textureBindGroup->get());

        computePass.DispatchWorkgroups(1, 1, 1);

        computePass.End();

        wgpu::CommandBuffer commands = encoder.Finish();

        queue->Submit(1, &commands);
    }