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
    enum class State {
        PathTracer,
        Rasterizer,
        Other
    };
    
public:
    Application(const std::string& applicationName);
    ~Application();

    void setDevice(wgpu::Device device_);
    void initializeAndRun();
    void transition(State state);

    void onUpdate();

    void renderFrame();

    TextureSystem* getTextureSystem() { return m_TextureSystem; }
    MaterialSystem* getMaterialSystem() { return m_MaterialSystem; }

    static Application* get();

private:
    void startPathTracer();
    void startRasterizer();

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

    State m_State;

    State m_TargetState;
    bool m_TransitionOnNextFrame;
    
};