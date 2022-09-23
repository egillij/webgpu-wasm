#pragma once

#include <string>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class WGpuDevice;
class WGpuSwapChain;
class WGpuVertexBuffer;
class WGpuIndexBuffer;
class WGpuUniformBuffer;
class WGpuShader;
class WGpuBindGroup;
class WGpuPipeline;
class WGpuTexture;
class WGpuSampler;

namespace wgpu {
    class Device;
}

class Application {
public:
    Application(const std::string& applicationName);
    ~Application();

    void setDevice(wgpu::Device device_);
    void initializeAndRun();

    void onUpdate();

    static Application* get();

private:
    std::string m_Name;

    WGpuDevice* m_Device = nullptr;

    bool m_IsInitialized;

    WGpuSwapChain* wSwapChain;
    WGpuVertexBuffer* vertexBuffer;
    WGpuIndexBuffer* indexBuffer;
    WGpuUniformBuffer* sceneUniformBuffer;
    WGpuUniformBuffer* modelUniformBuffer;
    WGpuUniformBuffer* modelUniformBuffer2;

    WGpuShader* shader = nullptr;
    WGpuBindGroup* sceneUniformBindGroup = nullptr;
    WGpuBindGroup* modelUniformBindGroup = nullptr;
    WGpuBindGroup* modelUniformBindGroup2 = nullptr;
    WGpuPipeline* pipeline = nullptr;
    WGpuTexture* texture;
    WGpuSampler* sampler;

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    struct SceneUniforms {
        glm::mat4 viewProjection;
    };

    struct ModelUniforms {
        glm::mat4 modelMatrix;
    };

    ModelUniforms modelUniforms;
    ModelUniforms modelUniforms2;
};