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

class CubemapBackgroundPipeline final {
public:
    CubemapBackgroundPipeline(WGpuDevice* device);
    ~CubemapBackgroundPipeline();

    void setCubemap(WGpuTexture* m_DepthTexture, WGpuCubemap* cubemap, WGpuDevice* device);

    void run(const glm::mat4& viewMatrix, WGpuDevice* device, WGpuTexture* target, wgpu::Queue* queue);

private:
    WGpuPipeline* m_Pipeline;

    WGpuShader* m_Shader;

    struct CameraUniforms {
        glm::mat4 projection;
        glm::mat4 viewRotation;
    };
    
    ///////////////////////////////////
    // Not owned by this pipeline
    WGpuCubemap* m_Cubemap;
    WGpuTexture* m_DepthTexture;
    ///////////////////////////////////

    WGpuBindGroupLayout* m_CameraUniformBindGroupLayout;
    WGpuBindGroup* m_CameraUniformBindGroup;
    WGpuBindGroupLayout* m_TextureBindGroupLayout;
    WGpuBindGroup* m_TextureBindGroup;
    WGpuSampler* m_NearestSampler;

    CameraUniforms m_CameraUniforms;
    WGpuUniformBuffer* m_CameraUniformBuffer;
};