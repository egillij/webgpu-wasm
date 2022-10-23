#include "PBRRenderPipeline.h"

#include "Renderer/Material.h"
#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuPipeline.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"

#include "Scene/GameObject.h"
#include "Renderer/Geometry/TriangleMesh.h"
#include "Renderer/Geometry/Part.h"

#include "Scene/Scene.h"

#include <set>

// Tímabundið á meðan fjöldi þríhyrninga er prentaður héðan
#include <emscripten.h>
#ifndef __EMSCRIPTEN__
#define EM_ASM(x, y)
#endif

static uint32_t totalTriangles = 0;
static uint32_t uniqueTriangles = 0;
static uint32_t uniqueObjects = 0;
static uint32_t uniqueParts = 0;

static const char shaderCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>,
        @location(2) fragPosition : vec3<f32>,
    }

    struct SceneUniforms {
        viewProjection : mat4x4<f32>,
        cameraPosition : vec3<f32>
    };

    struct ModelUniforms {
        modelMatrix : mat4x4<f32>,
        normalMatrix : mat4x4<f32>,
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
        let fragPosition = modelUniforms.modelMatrix * vec4<f32>(positionIn, 1.0);
        output.position = sceneUniforms.viewProjection * fragPosition;
        output.fragUV = uvIn;
        output.fragNormal = (modelUniforms.normalMatrix * vec4<f32>(normalIn, 0.0)).xyz;
        output.fragPosition = fragPosition.xyz;
        return output;
    }

    struct MaterialUniforms {
        albedo   : vec3<f32>,
        filler   : f32,
        ambient  : vec3<f32>,
        filler2  : f32,
        specular : vec3<f32>,
        shininess : f32,
    };

    struct MaterialParameters {
        albedo : vec3<f32>,
        metallic : f32,
        specular : vec3<f32>,
        roughness : f32,
    };

    @group(2) @binding(0) var<uniform> materialParameters : MaterialParameters;
    @group(2) @binding(1) var albedoTexture : texture_2d<f32>;
    @group(2) @binding(2) var metallicTexture : texture_2d<f32>;
    @group(2) @binding(3) var roughnessTexture : texture_2d<f32>;
    @group(2) @binding(4) var aoTexture : texture_2d<f32>;

    @group(3) @binding(0) var nearestSampler : sampler;

    const M_PI : f32 = 3.141592653589793;
    const M_1_PI : f32 = 0.318309886183791;

    fn fresnelSchlick(cosTheta : f32, F0 : vec3<f32>) -> vec3<f32> {
        //return F0 + (1.0 - F0) * pow(clamp(1.0-clampcosTheta, 0.0, 1.0), 5);
        return F0 + (1.0 - F0) * pow(1.0-cosTheta, 5);
    }

    fn DistributionGGX(NdotH : f32, roughness : f32) -> f32 {
        let a : f32 = roughness * roughness;
        let a2 : f32 = a * a;
        let NdotH2 : f32 = NdotH * NdotH;

        var den : f32 = NdotH2 * (a2 - 1.0) + 1.0;
        den = M_PI * den * den;

        return a2 / den;
    }

    fn GeometrySchlickGGX(NdotV : f32, roughness : f32) -> f32 {
        let r : f32 = roughness + 1.0;
        let k : f32 = r * r / 8.0;

        let den : f32 = NdotV * (1.0 - k) + k;

        return NdotV / den;
    }

    // V is vector from world position to eye
    // L is vector from world position to light
    fn GeometrySmith(NdotV : f32, NdotL : f32, roughness : f32) -> f32 {
        let ggx2 : f32 = GeometrySchlickGGX(NdotV, roughness);
        let ggx1 : f32 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    const lDir = vec3<f32>(0.0, 0.0, -1.0);
    const lCol = vec3<f32>(1.0, 1.0, 1.0);

    @fragment
    fn main_f(
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>,
        @location(2) fragPosition : vec3<f32>
    ) -> @location(0) vec4<f32> {
        
        var N = normalize(fragNormal);
        
        let V = normalize(fragPosition - sceneUniforms.cameraPosition);

        let N_dot_V = dot(N, -V);
        if(N_dot_V < 0.0)   {
            N = -N;
        }

        // let R = reflect(lightDir, norm);

        var albedo = materialParameters.albedo;
        let albedoSample : vec4<f32> = textureSample(albedoTexture, nearestSampler, fragUV);
        albedo = pow(albedoSample.rgb, vec3<f32>(2.2));

        let metallic : f32 = textureSample(metallicTexture, nearestSampler, fragUV).r;
        let roughness : f32 = textureSample(roughnessTexture, nearestSampler, fragUV).r;
        let ao : f32 = textureSample(aoTexture, nearestSampler, fragUV).r;

        var F0 : vec3<f32> = vec3<f32>(0.04);
        F0 = mix(F0, albedo, metallic);

        var Lo : vec3<f32> = vec3<f32>(0.0);

        // Lighting
        let L = lDir; // Already normalized

        let H : vec3<f32> = normalize(-V-L);

        let NdotL : f32 = max(0.0, dot(N, -L));
        let NdotV : f32 = max(0.0, dot(N, -V));
        let NdotH : f32 = max(0.0, dot(N, H));
        let HdotV : f32 = max(0.0, dot(H, -V));

        let radiance : vec3<f32> = lCol;

        let NDF : f32 = DistributionGGX(NdotH, roughness);
        let G : f32 = GeometrySmith(NdotV, NdotL, roughness);
        let F : vec3<f32> = fresnelSchlick(HdotV, F0);

        let kS : vec3<f32> = F;
        var kD : vec3<f32> = vec3<f32>(1.0) - kS;
        kD *= 1.0 - metallic;

        let num : vec3<f32> = NDF * G * F;
        let den : f32 = 4.0 * NdotV * NdotL + 0.0001;
        let specular = num / den;

        Lo = (M_1_PI * kD * albedo + specular) * radiance * NdotL;
        
        let ambient = ao * albedo;

        var color = ambient +  Lo;

        // Tone mapping
        color = color / (color + vec3(1.0));

        // Gamma correction
        let gamma : f32 = 1.0 / 2.2;
        let gammaVec : vec3<f32> = vec3<f32>(gamma, gamma, gamma);

        let fragColor = vec4<f32>(pow(color, gammaVec), 1.0);
        return fragColor;
    }
)";


PBRRenderPipeline::PBRRenderPipeline(uint32_t width, uint32_t height, WGpuDevice* device)
: RenderPipeline("PBR Render Pipeline")
{
    m_Pipeline = new WGpuPipeline();

    m_SceneUniformBindGroupLayout = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    m_SceneUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(SceneUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    m_SceneUniformBindGroupLayout->build(device);

    m_MaterialBindGroupLayout = new WGpuBindGroupLayout("Material Bind Group Layout");
    m_MaterialBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(PBRUniforms::ShaderUniforms), 0, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 1, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 2, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 3, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->addTexture(TextureSampleType::Float, 4, wgpu::ShaderStage::Fragment);
    m_MaterialBindGroupLayout->build(device);

    m_ModelUniformBindGroupLayout = new WGpuBindGroupLayout("Model Bind Group Layout");
    m_ModelUniformBindGroupLayout->addBuffer(BufferBindingType::Uniform, sizeof(ModelUniforms), 0, wgpu::ShaderStage::Vertex);
    m_ModelUniformBindGroupLayout->build(device);

    m_SamplerBindGroupLayout = new WGpuBindGroupLayout("Sampler Bind Group Layout");
    m_SamplerBindGroupLayout->addSampler(SamplerBindingType::NonFiltering, 0, wgpu::ShaderStage::Fragment);
    m_SamplerBindGroupLayout->build(device);

    m_Shader = new WGpuShader("Color Shader", shaderCode, device);

    m_Pipeline->addBindGroup(m_SceneUniformBindGroupLayout);
    m_Pipeline->addBindGroup(m_ModelUniformBindGroupLayout);
    m_Pipeline->addBindGroup(m_MaterialBindGroupLayout);
    m_Pipeline->addBindGroup(m_SamplerBindGroupLayout);
    m_Pipeline->setShader(m_Shader);
    m_Pipeline->build(device);

    TextureCreateInfo depthInfo{};
    depthInfo.format = TextureFormat::Depth32Float;
    depthInfo.height = height;
    depthInfo.width = width;
    depthInfo.usage = {TextureUsage::RenderAttachment};
    m_DepthTexture = new WGpuTexture("Depth texture", &depthInfo, device);

}

PBRRenderPipeline::~PBRRenderPipeline()
{
    if(m_Pipeline) delete m_Pipeline;
}

void PBRRenderPipeline::render(Scene* scene, WGpuDevice* device, WGpuSwapChain* swapChain)
{
    if(!scene) return;
    wgpu::Queue queue = device->getHandle().GetQueue();
    
    wgpu::TextureView backBuffer = swapChain->getCurrentFrameTexture();// swapChain.GetCurrentTextureView();

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

    totalTriangles = 0;

    static bool cacheTransforms = true;
    static std::set<uint32_t> uniqueIds;

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();
        {   
            wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);
            renderPass.SetPipeline(m_Pipeline->getPipeline());
            renderPass.SetBindGroup(0, scene->getUniformsBindGroup()->get());
            renderPass.SetBindGroup(3, scene->m_SamplerBindGroup->get());
            
            auto gameObjects = scene->getGameObjects();
            uniqueObjects = gameObjects.size();


            for(int i = 0; i < gameObjects.size(); ++i){
                GameObject* object = gameObjects.at(i);
                // if(uniqueIds.count(object->getId()) <= 0) {
                //     ++uniqueParts;
                // }

                // uniqueIds.insert(object->getId());

                glm::mat4 transform = object->getTransform();

                const TriangleMesh* mesh = object->getMesh();
                const Material* material = object->getMaterial();

                if(mesh != nullptr && material != nullptr && mesh->isReady()){
                    WGpuBindGroup* modelBindGroup = object->getModelBindGroup();
                    if(cacheTransforms)
                        object->cacheTransform(transform, device);

                    renderPass.SetBindGroup(1, modelBindGroup->get());
                        
                    renderPass.SetBindGroup(2, material->getBindGroup()->get());

                    renderPass.SetVertexBuffer(0, mesh->getVertexBuffer()->getHandle());

                    WGpuIndexBuffer* indexBuffer = mesh->getIndexBuffer();
                    renderPass.SetIndexBuffer(indexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(indexBuffer->getDataFormat()));
                    renderPass.DrawIndexed(indexBuffer->getIndexCount());
                    totalTriangles+= indexBuffer->getIndexCount() / 3;
                }

                GameObject* child = object->getNext();
                while(child != nullptr) {
                    transform = child->getTransform() * transform;
                    
                    const TriangleMesh* mesh = child->getMesh();
                    const Material* material = child->getMaterial();

                    if(mesh != nullptr && material != nullptr && mesh->isReady()){
                        WGpuBindGroup* modelBindGroup = child->getModelBindGroup();
                        if(cacheTransforms)
                            child->cacheTransform(transform, device);

                        renderPass.SetBindGroup(1, modelBindGroup->get());
                            
                        renderPass.SetBindGroup(2, material->getBindGroup()->get());

                        renderPass.SetVertexBuffer(0, mesh->getVertexBuffer()->getHandle());

                        WGpuIndexBuffer* indexBuffer = mesh->getIndexBuffer();
                        renderPass.SetIndexBuffer(indexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(indexBuffer->getDataFormat()));
                        renderPass.DrawIndexed(indexBuffer->getIndexCount());
                        totalTriangles += indexBuffer->getIndexCount() / 3;

                        if(uniqueIds.count(child->getId()) <= 0) {
                            ++uniqueParts;
                            uniqueTriangles += indexBuffer->getIndexCount() / 3;
                        }
                        
                        uniqueIds.insert(child->getId());
                    }

                    child = child->getNext();
                }
            }
            
            renderPass.End();
        }
        commands = encoder.Finish();
        frameNr++;
    }

    queue.Submit(1, &commands);
    cacheTransforms = false;

    // Tímabundið
    EM_ASM({
        var elm = document.getElementById("TotalTriangleCount");
        elm.innerHTML = $0;
        elm = document.getElementById("UniqueObjectCount");
        elm.innerHTML = $1;
        elm = document.getElementById("UniquePartCount");
        elm.innerHTML = $2;
        elm = document.getElementById("UniqueTriangleCount");
        elm.innerHTML = $3;
    }, totalTriangles, uniqueObjects, uniqueParts, uniqueTriangles);
}