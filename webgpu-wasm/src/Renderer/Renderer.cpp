#include "Renderer.h"

#include "Renderer/Geometry/TriangleMesh.h"
#include "Renderer/Geometry/Part.h"
#include "Renderer/Material.h"

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

    struct MaterialUniforms {
        color : vec3<f32>,
    };

    @group(2) @binding(0) var<uniform> materialUniforms : MaterialUniforms;
    @group(2) @binding(1) var mySampler : sampler;
    @group(2) @binding(2) var myTexture : texture_2d<f32>;

    @fragment
    fn main_f(
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>
    ) -> @location(0) vec4<f32> {
        var fragColor : vec4<f32> = textureSample(myTexture, mySampler, fragUV);
        let gamma : f32 = 1.0 / 2.2;
        let gammaVec : vec3<f32> = vec3<f32>(gamma, gamma, gamma);
        fragColor = vec4<f32>(pow(materialUniforms.color *fragColor.rgb, gammaVec), 1.0);
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
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
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

    int frameTriangles = 0;

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = m_Device->getHandle().CreateCommandEncoder();
        {   
            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(m_Pipeline->getPipeline());
            renderPass.SetBindGroup(0, scene->getUniformsBindGroup()->get());
            
            auto gameObjects = scene->getGameObjects();

            for(int i = 0; i < gameObjects.size(); ++i){
                GameObject* object = gameObjects.at(i);
                std::vector<Part*>& parts = object->getParts();

                WGpuBindGroup* modelBindGroup = object->getModelBindGroup();

                renderPass.SetBindGroup(1, modelBindGroup->get());
                renderPass.SetBindGroup(2, object->getMaterialBindGroup()->get());

                for(size_t j = 0; j < parts.size(); ++j){
                    TriangleMesh* mesh = parts[j]->getMesh();
                    if(!mesh->isReady()) {
                        continue;
                    }
                    
                    renderPass.SetVertexBuffer(0, mesh->getVertexBuffer()->getHandle());

                    WGpuIndexBuffer* indexBuffer = mesh->getIndexBuffer();
                    renderPass.SetIndexBuffer(indexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(indexBuffer->getDataFormat()));

                    renderPass.DrawIndexed(indexBuffer->getIndexCount());
                    frameTriangles += indexBuffer->getIndexCount() / 3;
                }
            }
            
            renderPass.End();
            
        }
        commands = encoder.Finish();
        frameNr++;
    }

    queue.Submit(1, &commands);
    // printf("Triangles drawn: %i\n", frameTriangles);
}