#pragma once

#include <string>
#include <vector>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class Scene;
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
class Renderer;
class PathTracer;

class MaterialSystem;
class GeometrySystem;
class TextureSystem;

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

    void startRendering();

    TextureSystem* getTextureSystem() { return m_TextureSystem; }
    MaterialSystem* getMaterialSystem() { return m_MaterialSystem; }

    static Application* get();

private:
    std::string m_Name;

    Scene* m_Scene = nullptr;
    Renderer* m_Renderer = nullptr;
    PathTracer* m_PathTracer = nullptr;

    MaterialSystem* m_MaterialSystem = nullptr;
    GeometrySystem* m_GeometrySystem = nullptr;
    TextureSystem* m_TextureSystem = nullptr;

    WGpuDevice* m_Device = nullptr;

    bool m_IsInitialized;
    
};