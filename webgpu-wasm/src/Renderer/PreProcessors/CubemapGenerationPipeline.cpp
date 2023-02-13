// Copyright 2023 Egill Ingi Jacobsen

#include "CubemapGenerationPipeline.h"

#include "CubemapGenerationShaders.h"

#include "Application.h"

#include "Renderer/Geometry/GeometrySystem.h"
#include "Renderer/Geometry/TriangleMesh.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuCubemap.h"
#include "Renderer/WebGPU/wgpuPipeline.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"
#include "Renderer/WebGPU/wgpuUniformBuffer.h"
#include "Renderer/WebGPU/wgpuSampler.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"

#include <glm/gtc/matrix_transform.hpp>

#define DELETE_IF_NOT_NULL(x) \
if(x == nullptr) {            \
    delete x;                 \
    x = nullptr;              \
}                             \

static glm::mat4 viewMatrices[6] = {
    glm::lookAt(glm::vec3(0.0f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f)),
};

CubemapGenerationPipeline::CubemapGenerationPipeline(PipelineType type, WGpuDevice* device)
: m_Device(device), m_Pipeline(nullptr), m_TextureBGL(nullptr), m_NearestSampler(nullptr),
  m_Shader(nullptr), m_CameraUniformBGL(nullptr)
{
    m_TextureBGL = new WGpuBindGroupLayout("Cubemap Vizualization Bind Group Layout");
    m_TextureBGL->addTexture(TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    m_TextureBGL->addSampler(SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    m_TextureBGL->build(m_Device);

    SamplerCreateInfo samplerInfo{};
    m_NearestSampler = new WGpuSampler("Sampler", &samplerInfo, m_Device);

    m_CameraUniformBGL = new WGpuBindGroupLayout("Scene Uniform Bind Group Layout");
    m_CameraUniformBGL->addBuffer(BufferBindingType::Uniform, sizeof(CameraUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
    m_CameraUniformBGL->build(m_Device);

    ShaderDescription pbrDescription{};
    pbrDescription.colorTargets = {TextureFormat::RGBA32Float};
    pbrDescription.type = ShaderType::VERTEX_FRAGMENT;
    switch(type){
        case PipelineType::EquirectangularToCubemap:{
            pbrDescription.shaderCode = equirectangularToCubemapCode;
            m_Shader = new WGpuShader("Equirectangluar to Cubemap", pbrDescription, device);
            break;
        }
        case PipelineType::DiffuseIrradiance:{
            pbrDescription.shaderCode = diffuseConvolutionCode;
            m_Shader = new WGpuShader("Diffuse Irradiance Convolution", pbrDescription, device);
            break;
        }
        case PipelineType::SpecularRadiance:{
            printf("Specular convolution not implemented yet");
            // pbrDescription.shaderCode = equirectangularToCubemapCode;
            // m_Shader = new WGpuShader("Equirectangluar to Cubemap", pbrDescription, device);
            break;
        }
    }
    

    m_Pipeline = new WGpuPipeline("Diffuse Irradiance Convolution");
    m_Pipeline->addBindGroup(m_CameraUniformBGL);
    m_Pipeline->addBindGroup(m_TextureBGL);
    m_Pipeline->setShader(m_Shader);
    m_Pipeline->build(device, true);

    m_CameraUniforms.projection = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 100.f);
    glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);

    wgpu::Queue queue = m_Device->getHandle().GetQueue();

    for(int i = 0; i < 6; ++i){
        m_CameraUniforms.view = viewMatrices[i];
        m_CameraUniformBuffer[i] = new WGpuUniformBuffer(device, "Diffuse Irradiance Camera UBO_" + std::to_string(i), sizeof(CameraUniforms));
        queue.WriteBuffer(m_CameraUniformBuffer[i]->getHandle(), 0, (void*) &m_CameraUniforms, sizeof(CameraUniforms));
        m_CameraUniformBindGroup[i] = new WGpuBindGroup("Diffuse Irradiance Camera Uniform BGL_" + std::to_string(i));
        m_CameraUniformBindGroup[i]->addBuffer(m_CameraUniformBuffer[i], BufferBindingType::Uniform, sizeof(CameraUniforms), 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
        m_CameraUniformBindGroup[i]->setLayout(m_CameraUniformBGL);
        m_CameraUniformBindGroup[i]->build(device);
    }
}

CubemapGenerationPipeline::~CubemapGenerationPipeline()
{
    DELETE_IF_NOT_NULL(m_Pipeline);
    DELETE_IF_NOT_NULL(m_TextureBGL);
    DELETE_IF_NOT_NULL(m_NearestSampler);
    for(int i = 0; i < 6; ++i){
        DELETE_IF_NOT_NULL(m_CameraUniformBindGroup[i]);
        DELETE_IF_NOT_NULL(m_CameraUniformBuffer[i]);
    }
    DELETE_IF_NOT_NULL(m_Shader);
    DELETE_IF_NOT_NULL(m_CameraUniformBGL);
}

void CubemapGenerationPipeline::process(WGpuTexture* input, WGpuCubemap* output)
{

    WGpuBindGroup textureBindGroup = WGpuBindGroup("Diffuse Irradiance Texture BG");
    textureBindGroup.setLayout(m_TextureBGL);
    textureBindGroup.addTexture(input, TextureSampleType::UnfilterableFloat, 0, wgpu::ShaderStage::Fragment);
    textureBindGroup.addSampler(m_NearestSampler, SamplerBindingType::NonFiltering, 1, wgpu::ShaderStage::Fragment);
    textureBindGroup.build(m_Device);

    wgpu::Queue queue = m_Device->getHandle().GetQueue();

    wgpu::CommandBuffer commands;
    
    {
        wgpu::CommandEncoder encoder = m_Device->getHandle().CreateCommandEncoder();
        for(int i = 0; i < 6; ++i)
        {   
            //TODO: do this for all faces of the cubemap. Change camera uniform buffer to have all the different view matrices. How to tell shader which one to use?
            // viewMatrix = glm::lookAt(glm::vec3(0.f), glm::normalize(direction), glm::vec3(0.f, 1.f, 0.f));
            // m_CameraUniforms.viewRotation = viewMatrix;
            // queue.WriteBuffer(m_CameraUniformBuffer->getHandle(), 0, (void*) &m_CameraUniforms, sizeof(CameraUniforms));

            wgpu::RenderPassColorAttachment attachment{};
            attachment.view = output->createView(static_cast<CubemapFace>(i));
            attachment.loadOp = wgpu::LoadOp::Clear;
            attachment.storeOp = wgpu::StoreOp::Store;
            attachment.clearValue = {0, 0, 0, 1};


            wgpu::RenderPassDescriptor renderPassDescription{};
            renderPassDescription.colorAttachmentCount = 1;
            renderPassDescription.colorAttachments = &attachment;
            
            uint32_t xStep = 100u;
            uint32_t yStep = 100u;

            uint32_t width = output->getWidth();
            uint32_t height = output->getHeight();

            uint32_t xSize = width / xStep;
            uint32_t ySize = height / yStep;

            std::vector<wgpu::CommandBuffer> commandVector;
            commandVector.resize((ySize+1)*(xSize+1));

            // Do the convolution in tiles to avoid timeouts
            for(uint32_t y = 0u; y < ySize + 1; ++y){
                for(uint32_t x = 0u; x < xSize+1; ++x){
                    if(x != 0) attachment.loadOp = wgpu::LoadOp::Load;
                    // wgpu::CommandEncoder encoder = m_Device->getHandle().CreateCommandEncoder();
                    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescription);

                    uint32_t xStart = x * xStep;
                    uint32_t yStart = y * yStep;
                    uint32_t actualXSize = std::fmin(xStep, width-xStart);
                    uint32_t actualYSize = std::fmin(yStep, height - yStart);

                    renderPass.SetScissorRect(xStart, yStart, actualXSize, actualYSize);
                    renderPass.SetPipeline(m_Pipeline->getPipeline());
                    renderPass.SetBindGroup(0, m_CameraUniformBindGroup[i]->get());
                    renderPass.SetBindGroup(1, textureBindGroup.get());

                    static GeometrySystem* geos = Application::get()->getGeometrySystem();
                    static const TriangleMesh* cube = geos->getCube();
                    renderPass.SetVertexBuffer(0, cube->getVertexBuffer()->getHandle());
                    static WGpuIndexBuffer* cubeIndexBuffer = cube->getIndexBuffer();
                    renderPass.SetIndexBuffer(cubeIndexBuffer->getHandle(), static_cast<wgpu::IndexFormat>(cubeIndexBuffer->getDataFormat()));
                    renderPass.DrawIndexed(cubeIndexBuffer->getIndexCount());
                    renderPass.End();
                    // commandVector.at(x + y * (xSize+1)) = encoder.Finish();
                }
            }
            // queue.Submit(commandVector.size(), commandVector.data());
        }
        commands = encoder.Finish();
    }
    
    queue.Submit(1, &commands);
}