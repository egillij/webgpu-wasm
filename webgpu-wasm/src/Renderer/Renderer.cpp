#include "Renderer.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuPipeline.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"

#include "Scene/GameObject.h"
#include "Scene/Scene.h"

#include "Utils/UniformStructs.h"

#include <emscripten/emscripten.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

static const char shaderCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>,
    }

    struct SceneUniforms {
        viewProjection : mat4x4<f32>,
    };

    struct ModelUniforms {
        modelMatrix : mat4x4<f32>,
    };

    @group(0) @binding(0) var<uniform> sceneUniforms : SceneUniforms;
    @group(1) @binding(0) var<uniform> modelUniforms : ModelUniforms;

    @vertex
    fn main_v(
        @location(0) positionIn : vec3<f32>,
        @location(1) normalIn : vec3<f32>,
        @location(2) uvIn : vec2<f32>
    ) -> VertexOutput {

        var output : VertexOutput;
        output.position = sceneUniforms.viewProjection * modelUniforms.modelMatrix * vec4<f32>(positionIn, 1.0);
        output.fragUV = uvIn;
        output.fragNormal = normalIn;
        return output;
    }

    @group(2) @binding(0) var mySampler : sampler;
    @group(2) @binding(1) var myTexture : texture_2d<f32>;

    @fragment
    fn main_f(
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>
    ) -> @location(0) vec4<f32> {
        var fragColor : vec4<f32> = textureSample(myTexture, mySampler, fragUV);
        let gamma : f32 = 1.0 / 2.2;
        let gammaVec : vec3<f32> = vec3<f32>(gamma, gamma, gamma);
        fragColor = vec4<f32>(pow(fragColor.rgb, gammaVec), 1.0);
        return fragColor;
    }
)";

Renderer::Renderer(uint32_t width, uint32_t height, WGpuDevice* device)
: m_Device(device)
{
    m_SwapChain = new WGpuSwapChain(m_Device, width, height);

    TextureCreateInfo depthInfo{};
    depthInfo.format = TextureFormat::Depth32Float;
    depthInfo.height = height;
    depthInfo.width = width;
    depthInfo.usage = {TextureUsage::RenderAttachment};
    m_DepthTexture = new WGpuTexture("Depth texture", &depthInfo, device);

    m_SceneUniformBindGroupLayout = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    m_SceneUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(SceneUniforms), 0, wgpu::ShaderStage::Vertex);
    m_SceneUniformBindGroupLayout->build(m_Device);

    m_MaterialBindGroupLayout = new WGpuBindGroupLayout("Material Bind Group Layout");
    m_MaterialBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(m_Device);

    m_ModelUniformBindGroupLayout = new WGpuBindGroupLayout("Model Bind Group Layout");
    m_ModelUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(ModelUniforms), 0, wgpu::ShaderStage::Vertex);
    m_ModelUniformBindGroupLayout->build(m_Device);

    shader = new WGpuShader("Color Shader", shaderCode, device);

    m_Pipeline = new WGpuPipeline();

    m_Pipeline->addBindGroup(m_SceneUniformBindGroupLayout);
    m_Pipeline->addBindGroup(m_ModelUniformBindGroupLayout);
    m_Pipeline->addBindGroup(m_MaterialBindGroupLayout);
    m_Pipeline->setShader(shader);
    m_Pipeline->build(device);


    /////////////////////////////////
    // Load texture
    emscripten_wget("/webgpu-wasm/avatar.jpg", "./avatar.jpg");
    stbi_set_flip_vertically_on_load(true);
    int texwidth, texheight, texchannels;
    unsigned char* imageData = stbi_load("./avatar.jpg", &texwidth, &texheight, &texchannels, 4);

    // wgpu::TextureDescriptor texDesc{};
    // texDesc.label = "Test texture";
    // texDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::Extent3D texExtent{};
    texExtent.width = texwidth;
    texExtent.height = texheight;
    // texDesc.size = texExtent;
    // texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;

    TextureCreateInfo texInfo{};
    texInfo.format = TextureFormat::RGBA8Unorm;
    texInfo.width = texwidth;
    texInfo.height = texheight;
    texInfo.usage = {TextureUsage::CopyDst, TextureUsage::TextureBinding};

    texture = new WGpuTexture("Test texture", &texInfo, device);

    SamplerCreateInfo samplerInfo{};
    sampler = new WGpuSampler("Sampler", &samplerInfo, device);
    // testTexture = wDevice.getHandle().CreateTexture(&texDesc);

    wgpu::Queue queue = device->getHandle().GetQueue();
    wgpu::ImageCopyTexture imgCpyTex{};
    imgCpyTex.texture = texture->getHandle();
    wgpu::Origin3D texOrig{};
    imgCpyTex.origin = texOrig;

    wgpu::TextureDataLayout texDataLayout{};
    texDataLayout.bytesPerRow = texwidth * 4;
    texDataLayout.rowsPerImage = texheight;
    texDataLayout.offset = 0;

    queue.WriteTexture(&imgCpyTex, imageData, texwidth*texheight*4, &texDataLayout, &texExtent);

    materialBindGroupLayout = new WGpuBindGroupLayout("Material Bind Group Layout");
    materialBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    materialBindGroupLayout->addTexture(TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    materialBindGroupLayout->build(m_Device);

    materialBindGroup = new WGpuBindGroup("Material Bind Group");
    materialBindGroup->setLayout(materialBindGroupLayout);
    materialBindGroup->addSampler(sampler, SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    materialBindGroup->addTexture(texture, TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    materialBindGroup->build(m_Device);
}

Renderer::~Renderer()
{
    
}

void Renderer::render(Scene* scene)
{
    if(!scene) return; //TODO: report error?

    wgpu::Queue queue = m_Device->getHandle().GetQueue();
    
    wgpu::TextureView backBuffer = m_SwapChain->getCurrentFrameTexture();// swapChain.GetCurrentTextureView();

    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = backBuffer;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    attachment.clearValue = {0, 0, 0, 1};
 
    wgpu::RenderPassDepthStencilAttachment depthAttachment{};
    
    wgpu::TextureViewDescriptor depthViewDesc{};
    depthViewDesc.label = "Depth View",
    depthViewDesc.format = wgpu::TextureFormat::Depth32Float;
    depthViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    depthViewDesc.mipLevelCount = 1;
    depthViewDesc.arrayLayerCount = 1;

    depthAttachment.view = m_DepthTexture->getHandle().CreateView(&depthViewDesc);
    depthAttachment.depthClearValue = 1.f;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthAttachment.depthReadOnly = false;

    wgpu::RenderPassDescriptor renderPassDescription{};
    renderPassDescription.colorAttachmentCount = 1;
    renderPassDescription.colorAttachments = &attachment;
    renderPassDescription.depthStencilAttachment = &depthAttachment;

    static uint64_t frameNr = 0;

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = m_Device->getHandle().CreateCommandEncoder();
        {   
            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(m_Pipeline->getPipeline());
            renderPass.SetBindGroup(0, scene->getUniformsBindGroup()->get());
            WGpuIndexBuffer* iBuffer = nullptr;
            WGpuVertexBuffer* vBuffer = nullptr;
            
            auto gameObjects = scene->getGameObjects();

            for(int i = 0; i < gameObjects.size(); ++i){
                GameObject* object = gameObjects.at(i);
                TriangleMesh* mesh = object->getMesh();
                if(!mesh) continue;
                
                WGpuBindGroup* modelBindGroup = object->getModelBindGroup();
                renderPass.SetVertexBuffer(0, mesh->getVertexBuffer()->getHandle());

                WGpuIndexBuffer* indexBuffer = mesh->getIndexBuffer();
                renderPass.SetIndexBuffer(indexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(indexBuffer->getDataFormat()));

                renderPass.SetBindGroup(1, modelBindGroup->get());
                renderPass.SetBindGroup(2, materialBindGroup->get());

                renderPass.DrawIndexed(indexBuffer->getIndexCount());
            }
            
            renderPass.End();
            
        }
        commands = encoder.Finish();
        frameNr++;
    }

    queue.Submit(1, &commands);   
}