#pragma once

#include "Renderer/Pipelines/RenderPipeline.h"

#include <glm/mat4x4.hpp>

class Scene;

class WGpuBindGroupLayout;
class WGpuBindGroup;
class WGpuDevice;
class WGpuShader;
class WGpuSampler;
class WGpuCubemap;
class WGpuUniformBuffer;

class CubemapVizualizationPipeline final {
public:
    CubemapVizualizationPipeline(uint32_t width, uint32_t height, WGpuDevice* device);
    ~CubemapVizualizationPipeline();

    void setCubemap(WGpuCubemap* cubemap, WGpuDevice* device);

    void run(WGpuDevice* device, WGpuSwapChain* swapChain);

private:
    WGpuPipeline* m_Pipeline;

    WGpuShader* m_Shader;

    struct CameraUniforms {
        glm::mat4 projection;
        glm::mat4 viewRotation;
    };
    
    WGpuCubemap* m_Cubemap;
    WGpuTexture* m_DepthTexture;
    WGpuBindGroupLayout* m_CameraUniformBindGroupLayout;
    WGpuBindGroup* m_CameraUniformBindGroup;
    WGpuBindGroupLayout* m_TextureBindGroupLayout;
    WGpuBindGroup* m_TextureBindGroup;
    WGpuSampler* m_NearestSampler;

    glm::mat4 viewMatrix;
    CameraUniforms m_CameraUniforms;
    WGpuUniformBuffer* m_CameraUniformBuffer;
};