#include "PathTracingPipeline.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/ComputePipeline.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"

static const char pathTracerCode[] = R"(

const imageWidth : f32 = 500.0;
const imageHeight : f32 = 500.0;
const M_PI : f32 = 3.141592653589793;
const M_1_PI : f32 = 0.318309886183790;
const DEG_TO_RAD = M_PI / 180.0;

fn noise(x : u32, y : u32) -> f32 {
    var n : u32 = x + y * 57;
    
    n = (n << 13) ^ n;

    let temp : u32 = (n * ((n * n * 15731) + 789221) +  1376312589) & 0x7fffffff; 
    let n_f : f32 = f32(temp); 
    return (1.0 - n_f / 1073741824.0);
}

struct Camera {
    position : vec3<f32>,
    direction : vec3<f32>,
    focalLength : f32,
    viewMatrix : mat4x4<f32>,
    fovY : f32,
    projectionMatrix : mat4x4<f32>,
};

fn generateRay(camera : Camera, x : f32, y : f32) -> Ray {
    let imageAspectRatio : f32 = imageWidth / imageHeight;  //assuming width > height 
    let Px : f32 = (2.0 * ((x + 0.5) / imageWidth) - 1.0) * tan(camera.fovY / 2.0 * DEG_TO_RAD) * imageAspectRatio; 
    let Py : f32 = (1.0 - 2.0 * ((y + 0.5) / imageHeight)) * tan(camera.fovY / 2.0 * DEG_TO_RAD); 

    var ray : Ray;
    ray.origin = vec3<f32>(0.0, 0.0, 0.0);

    // var cameraToWorld : mat4<f32>; 
    // cameraToWorld.set(...);  //set matrix 
    // Vec3f rayOriginWorld, rayPWorld; 
    // cameraToWorld.multVectMatrix(rayOrigin, rayOriginWorld); 
    // cameraToWorld.multVectMatrix(Vec3f(Px, Py, -1), rayPWorld); 
    // Vec3f rayDirection = rayPWorld - rayOriginWorld; 
    // rayDirection.normalize();  //it's a direction so don't forget to normalize

    ray.direction = vec3<f32>(Px, Py, -1) - ray.origin;
    ray.direction = normalize(ray.direction);
    return ray;
}

struct Ray {
    origin : vec3<f32>,
    direction : vec3<f32>,
};

struct Payload {
    position : vec3<f32>,
    normal : vec3<f32>,
    hit : bool,
    distance: f32,
};

struct Sphere {
    center : vec3<f32>,
    radius : f32,
};

fn raySphereIntersection(ray : Ray, sphere : Sphere) -> Payload {
    var t0 : f32 = -1.0;
    var t1 : f32 = -1.0;

    let radius2 : f32 = sphere.radius * sphere.radius;

    var pl : Payload;
    pl.position = vec3<f32>(0.0, 0.0, 0.0);
    pl.normal = vec3<f32>(0.0, 11.0, 0.0);
    pl.hit = false;

    let L : vec3<f32> = ray.origin - sphere.center; 
    let a : f32 = dot(ray.direction, ray.direction);
    let b : f32 = 2 * dot(ray.direction, L); 
    let c : f32 = dot(L, L) - radius2; 
    // if (!solveQuadratic(a, b, c, t0, t1)) return false; 

    // Solve quadratic. TODO: make as a function
    let discr : f32 = b * b - 4 * a * c; 
    if (discr < 0) {
        return pl; 
    }
    else if (discr == 0) {
        t0 = -0.5 * b / a; 
        t1 = t0;
    }
    else { 
        var q : f32 = 0.0;
        if(b > 0) {
            q = -0.5 * (b + sqrt(discr));
        }
        else {
            q = -0.5 * (b - sqrt(discr)); 
        }
               
        t0 = q / a; 
        t1 = c / q; 
    } 

    if (t0 > t1) {
        let temp : f32 = t0;
        t0 = t1;
        t1 = temp;
    } 

    if (t0 < 0) { 
        t0 = t1;  //if t0 is negative, let's use t1 instead 
        if (t0 < 0) {
            return pl;  //both t0 and t1 are negative 
        }
    } 

    pl.hit = true;
    pl.position = ray.origin + t0 * ray.direction;
    pl.normal = normalize(pl.position - sphere.center);
    pl.distance = t0;

    return pl; 
}

struct PointLight {
    position : vec3<f32>,
    radiance : vec3<f32>,
};

fn shadeDiffuse(payload : Payload, pointLight : PointLight) -> vec4<f32> {
    let L : vec3<f32> = normalize(pointLight.position - payload.position);
    let cos_wi = max(0.0, dot(payload.normal, L));

    var color : vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    let albedo : vec3<f32> = vec3<f32>(0.7, 0.7, 0.7);
    color = vec4<f32>(albedo * M_1_PI * pointLight.radiance * cos_wi + 0.01, 1.0);
    return color;
}


@group(0) @binding(0) var targetTexture : texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(1, 1, 1)
fn main() {

  var camera : Camera;
  camera.position = vec3<f32>(0.0, 0.0, 2.0);
  camera.direction = vec3<f32>(0.0, 0.0, -1.0);
  camera.focalLength = 2.0; // Hvað er góð stilling hér?
  camera.fovY = 45.0;

  var sphere : Sphere;
  sphere.center = vec3<f32>(0.0, 0.0, -5.0);
  sphere.radius = 1.0;

  var pointLight : PointLight;
  pointLight.position = vec3<f32>(-3.f, 1.f, -3.f);
  pointLight.radiance = vec3<f32>(0.2, 0.4, 0.6);

  for(var x : u32 = 0; x < 500; x++) {
    for(var y : u32 = 0; y < 500; y++) {

        let ray : Ray = generateRay(camera, f32(x), f32(y));
        
        var pl : Payload = raySphereIntersection(ray, sphere);

        var color : vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, 1.0);
        if(pl.hit) {
            color = shadeDiffuse(pl, pointLight);
        }
        else {
            color = vec4<f32>(noise(x, y), noise(2*x, 2*y), noise(4*x, 4*y), 1.0);
        }

        let uv : vec2<u32> = vec2<u32>(x,y);
        textureStore(targetTexture, uv, color);
    }
  }
}
)";

PathTracingPipeline::PathTracingPipeline()
: m_IsValid(false)
{

}

PathTracingPipeline::PathTracingPipeline(WGpuDevice* device)
{
    TextureCreateInfo info{};
    info.format = TextureFormat::RGBA8Unorm;
    info.width = 500;
    info.height = 500;
    info.usage = {TextureUsage::StorageBinding, TextureUsage::TextureBinding, TextureUsage::CopyDst};
    m_TargetTexture = new WGpuTexture("PathTracing Target", &info, device);

    ShaderDescription shaderDesc{};
    shaderDesc.type = ShaderType::COMPUTE;
    shaderDesc.shaderCode = pathTracerCode;
    m_PathTraceShader = new WGpuShader("Path Tracing Shader", shaderDesc, device);

    m_BindGroupLayout = new WGpuBindGroupLayout("Compute Test BGL");
    m_BindGroup = new WGpuBindGroup("Compute Test BG");

    m_BindGroupLayout->addStorageTexture(wgpu::StorageTextureAccess::WriteOnly, TextureFormat::RGBA8Unorm, wgpu::TextureViewDimension::e2D, 0, wgpu::ShaderStage::Compute);
    m_BindGroupLayout->build(device);

    m_BindGroup->setLayout(m_BindGroupLayout);
    m_BindGroup->addStorageTexture(m_TargetTexture, 0, wgpu::ShaderStage::Compute);

    m_BindGroup->build(device);
    
    m_Pipeline = new ComputePipeline("Test Compute");
    m_Pipeline->setShader(m_PathTraceShader);
    m_Pipeline->addBindGroup(m_BindGroupLayout);
    m_Pipeline->build(device);

    m_IsValid = true;
    
}

PathTracingPipeline::~PathTracingPipeline()
{
}

void PathTracingPipeline::run(WGpuDevice* device)
{
    if(!m_IsValid) return;

    wgpu::Queue queue = device->getHandle().GetQueue();

    wgpu::ComputePassDescriptor desc{};
    desc.label = "Path Tracer pass descriptor";

    wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();

    wgpu::ComputePassEncoder computePass = encoder.BeginComputePass(&desc);
    computePass.SetPipeline(m_Pipeline->getPipeline());
    computePass.SetLabel("Path Tracing Pass");
    computePass.SetBindGroup(0, m_BindGroup->get());

    computePass.DispatchWorkgroups(1, 1, 1);

    computePass.End();

    wgpu::CommandBuffer commands = encoder.Finish();

    queue.Submit(1, &commands);
}

WGpuTexture* PathTracingPipeline::getTargetBuffer()
{
    return m_TargetTexture;
}