// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <glm/mat4x4.hpp>

class WGpuDevice;
class WGpuTexture;
class WGpuCubemap;
class WGpuPipeline;
class WGpuBindGroupLayout;
class WGpuUniformBuffer;
class WGpuSampler;
class WGpuBindGroup;
class WGpuShader;

// Take a High-Dynamic Range image and uses convolution
// to generate a diffuse irradiance cubemap
class CubemapGenerationPipeline {
public:
    enum class PipelineType {
        EquirectangularToCubemap = 0,
        DiffuseIrradiance,
        SpecularRadiance
    };
public:
    CubemapGenerationPipeline(PipelineType type, WGpuDevice* device);
    ~CubemapGenerationPipeline();

    void process(WGpuTexture* input, WGpuCubemap* output);

private:
    WGpuDevice* m_Device;
    WGpuPipeline* m_Pipeline;
    WGpuBindGroupLayout* m_TextureBGL;
    WGpuSampler* m_NearestSampler;

    WGpuShader* m_Shader;

    struct CameraUniforms {
        glm::mat4 projection;
        glm::mat4 view;
    };
    CameraUniforms m_CameraUniforms;
    WGpuUniformBuffer* m_CameraUniformBuffer[6];
    WGpuBindGroupLayout* m_CameraUniformBGL;
    WGpuBindGroup* m_CameraUniformBindGroup[6];

};