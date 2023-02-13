// Copyright 2023 Egill Ingi Jacobsen

#include "PathTracingPipeline.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuSwapChain.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuShader.h"
#include "Renderer/WebGPU/ComputePipeline.h"
#include "Renderer/WebGPU/wgpuBindGroup.h"

#include <emscripten/html5.h>
#ifndef __EMSCRIPTEN__
#define EM_ASM(x, y)
#endif

static const char pathTracerCode[] = R"(

const imageWidth : f32 = 500.0;
const imageHeight : f32 = 500.0;
const M_PI : f32 = 3.141592653589793;
const M_1_PI : f32 = 0.318309886183790;
const DEG_TO_RAD : f32 = M_PI / 180.0;

const MAX_RAY_DEPTH : u32 = 7;

struct Camera {
    position : vec3<f32>,
    direction : vec3<f32>,
    focalLength : f32,
    viewMatrix : mat4x4<f32>,
    fovY : f32,
    projectionMatrix : mat4x4<f32>,
};

struct Ray {
    origin : vec3<f32>,
    direction : vec3<f32>,
};

struct Payload {
    // position, normal, hit, distance, materialIndex of current trace
    position : vec3<f32>,
    geometricNormal : vec3<f32>,
    shadingNormal : vec3<f32>,
    hit : bool,
    distance: f32,
    materialIndex : u32,

    // Should the ray bounce again
    bounce : bool,
    // Current ray depth (number of bounces)
    raydepth: u32,

    // Cumulative attenuation of the color
    attenuation: vec3<f32>,

    ior: f32,
    inside : bool,

    // ray direction for next bounce
    nextDirection: vec3<f32>,
    nextPosition: vec3<f32>,
};

struct PointLight {
    position : vec3<f32>,
    radiance : vec3<f32>,
};

const DIFFUSE_MATERIAL : u32 = 0;
const METAL_MATERIAL : u32 = 0;

struct Material {
    albedo : vec3<f32>,
    roughness : f32,
    metalness : f32,
    transparent : bool,
    ior : f32,
};

struct Sphere {
    center : vec3<f32>,
    radius : f32,
};

struct Object {
    //TODO: hafa position, rotation og scale
    primitiveIndex: u32,
    materialIndex : u32,
};

fn noise(x : u32, y : u32) -> f32 {
    var n : u32 = x + y * 57;
    
    n = (n << 13) ^ n;

    let temp : u32 = (n * ((n * n * 15731) + 789221) +  1376312589) & 0x7fffffff; 
    let n_f : f32 = f32(temp); 
    return (1.0 - n_f / 1073741824.0);
}

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

fn raySphereIntersection(ray : Ray, sphere : Sphere) -> Payload {
    var t0 : f32 = -1.0;
    var t1 : f32 = -1.0;

    let radius2 : f32 = sphere.radius * sphere.radius;

    var pl : Payload;
    pl.position = vec3<f32>(0.0, 0.0, 0.0);
    pl.geometricNormal = vec3<f32>(0.0, 1.0, 0.0);
    pl.shadingNormal = pl.geometricNormal;
    pl.hit = false;
    pl.inside = false;

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
    pl.geometricNormal = normalize(pl.position - sphere.center);
    pl.shadingNormal = pl.geometricNormal;
    pl.inside = false;

    if(dot(ray.direction, pl.shadingNormal) > 0) {
        pl.shadingNormal = -pl.shadingNormal;
        pl.inside = true;
    }

    pl.distance = t0;

    return pl; 
}

fn shadeRay(payload : ptr<function, Payload>, pointLight : PointLight, material : ptr<function, Material>) -> vec4<f32> {
    var shadedColor : vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    var L : vec3<f32> = normalize(pointLight.position - (*payload).position);
    var cos_wi = max(0.0, dot((*payload).shadingNormal, L));
    if((*material).metalness < 0.5){
        shadedColor = vec4<f32>((*payload).attenuation * (*material).albedo * M_1_PI * (pointLight.radiance * cos_wi + 0.05), 1.0);
    }
    else {
        shadedColor = vec4<f32>((*material).albedo, 1.0);
    }
    return shadedColor;
}

@group(0) @binding(0) var targetTexture : texture_storage_2d<rgba8unorm, write>;

// @compute @workgroup_size(1, 1, 1)
@compute @workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) invocationId : vec3<u32> ) {
    ////////////////////////////////////////////////////////
    //Scene. Gera þetta öðruvísis svo þetta sé ekki búið til í hvert skipti
    const NUM_MATERIALS = 5;
    const NUM_SPHERES = 6;
    const NUM_OBJECTS = 6;
    var materialList : array<Material, NUM_MATERIALS>;
    var sphereList : array<Sphere, NUM_SPHERES>;
    var objectList : array<Object, NUM_OBJECTS>;

    var camera : Camera;
    camera.position = vec3<f32>(0.0, 0.0, 2.0);
    camera.direction = vec3<f32>(0.0, 0.0, -1.0);
    camera.focalLength = 2.0; // Hvað er góð stilling hér?
    camera.fovY = 45.0;

    var sceneLight : PointLight;
    sceneLight.position = vec3<f32>(-3.f, 1.f, -3.f);
    sceneLight.radiance = vec3<f32>(1.0, 1.0, 1.0);

    var material1 : Material;
    material1.albedo = vec3<f32>(0.5, 0.5, 0.9);
    material1.roughness = 0.0;
    material1.metalness = 1.0;
    material1.transparent = false;
    material1.ior = 1.414;
    materialList[0] = material1;

    var material2 : Material;
    material2.albedo = vec3<f32>(0.63, 0.1, 0.2);
    material2.roughness = 0.0;
    material2.metalness = 0.0;
    material2.transparent = false;
    material2.ior = 1.414;
    materialList[1] = material2;

    var material3 : Material;
    material3.albedo = vec3<f32>(0.23, 0.92, 0.14);
    material3.roughness = 0.8;
    material3.metalness = 0.0;
    material3.transparent = false;
    material3.ior = 1.414;
    materialList[2] = material3;

    var material4 : Material;
    material4.albedo = vec3<f32>(0.23, 0.42, 0.94);
    material4.roughness = 0.4;
    material4.metalness = 0.0;
    material4.transparent = false;
    material4.ior = 1.414;
    materialList[3] = material4;

    var material5 : Material;
    material5.albedo = vec3<f32>(0.9, 0.5, 0.5);// vec3<f32>(0.87, 0.35, 0.17);
    material5.roughness = 0.4;
    material5.metalness = 0.0;
    material5.transparent = true;
    material5.ior = 1.414;
    materialList[4] = material5;

    var sphere : Sphere;
    sphere.center = vec3<f32>(0.0, 0.0, -5.0);
    sphere.radius = 1.0;
    sphereList[0] = sphere;

    var sphere2 : Sphere;
    sphere2.center = vec3<f32>(-1.0, 1.0, -2.0);
    sphere2.radius = 0.5;
    sphereList[1] = sphere2;

    var sphere3 : Sphere;
    sphere3.center = vec3<f32>(1.0, -1.0, -2.0);
    sphere3.radius = 1.0;
    sphereList[2] = sphere3;

    var sphere4 : Sphere;
    sphere4.center = vec3<f32>(0.0, -101.0, -2.0);
    sphere4.radius = 100.0;
    sphereList[3] = sphere4;

    var sphere5 : Sphere;
    sphere5.center = vec3<f32>(-1.0, -0.5, -3.0);
    sphere5.radius = 0.5;
    sphereList[4] = sphere5;


    var sphere6 : Sphere;
    sphere6.center = vec3<f32>(0.0, 0.0, 1000.0);
    sphere6.radius = 990.0;
    sphereList[5] = sphere6;


    var object1 : Object;
    object1.primitiveIndex = 0;
    object1.materialIndex = 0;
    objectList[0] = object1;

    var object2 : Object;
    object2.primitiveIndex = 1;
    object2.materialIndex = 1;
    objectList[1] = object2;

    var object3 : Object;
    object3.primitiveIndex = 2;
    object3.materialIndex = 3;
    objectList[2] = object3;

    var object4 : Object;
    object4.primitiveIndex = 3;
    object4.materialIndex = 2;
    objectList[3] = object4;

    var object5 : Object;
    object5.primitiveIndex = 4;
    object5.materialIndex = 4;
    objectList[4] = object5;

    var object6 : Object;
    object6.primitiveIndex = 5;
    object6.materialIndex = 3;
    objectList[5] = object6;

    //////////////////////////////////////////////////////

    var xRange : u32 = 500 / 8;
    var yRange : u32 = 500 / 8;
    var xStart : u32 = 500 / 8 * invocationId.x;
    var yStart : u32 = 500 / 8 * invocationId.y;

    for(var x : u32 = xStart; x < xStart+xRange; x++) {
        for(var y : u32 = yStart; y < yStart+yRange; y++) {
            var closestPayload : Payload;
            closestPayload.hit = false;
            closestPayload.distance = -1.0;
            closestPayload.raydepth = 0;
            closestPayload.attenuation = vec3<f32>(1.0, 1.0, 1.0);
            closestPayload.ior = 1.0;
            closestPayload.inside = false;

            var pixelColor : vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, 1.0);

            for(var r : u32 = 0; r < MAX_RAY_DEPTH; r++){
                closestPayload.hit = false;
                var ray : Ray;
                if(r == 0) {
                    // The first ray is always a camera ray
                    ray = generateRay(camera, f32(x), f32(y));
                }
                else {
                    // Subsequent rays are either reflections or refractions
                    // TODO: payload ætti að halda utan um direction fyrir nýjan ray
                    ray.origin = closestPayload.nextPosition;
                    ray.direction = closestPayload.nextDirection;
                }    
                
                for(var i : u32 = 0; i < NUM_OBJECTS; i++) {
                    //TODO: nota pointer á payload og skila bool hvort það var hit
                    var pl : Payload = raySphereIntersection(ray, sphereList[objectList[i].primitiveIndex]);
                    if(pl.hit && (!closestPayload.hit || pl.distance < closestPayload.distance)){
                        pl.attenuation = closestPayload.attenuation;
                        pl.ior = closestPayload.ior;
                        // pl.inside = closestPayload.inside;
                        closestPayload = pl;
                        closestPayload.materialIndex = objectList[i].materialIndex;
                    }
                }

                if(closestPayload.hit) {
                    //TODO: fara yfir öll ljós og taka meðaltal. Gera shadow ray trace til að komast að því hvort að ljósið sé sjáanlegt
                    var currentMaterial = materialList[closestPayload.materialIndex];

                    //TODO: ákvarða út frá hvaða material var hitt. T.d. þarf diffuse ekki bounce
                    if(currentMaterial.transparent){
                        closestPayload.bounce = true;

                        closestPayload.nextDirection = refract(ray.direction, closestPayload.shadingNormal, closestPayload.ior / currentMaterial.ior );
                        closestPayload.nextPosition = closestPayload.position - closestPayload.shadingNormal * 0.001;
                        closestPayload.attenuation *= currentMaterial.albedo;
                        if(closestPayload.inside) {
                            closestPayload.ior = 1.0;
                        }
                        else {
                            closestPayload.ior = currentMaterial.ior;
                        }
                        // closestPayload.bounce = false;
                        // pixelColor += vec4<f32>(1.0, 0.0, 1.0, 1.0);
                        // pixelColor = vec4<f32>(1.0, 0.0, 0.0, 1.0);
                    }
                    else if(currentMaterial.metalness > 0.9) {
                        closestPayload.bounce = true;
                        closestPayload.nextDirection = reflect(ray.direction, closestPayload.shadingNormal);
                        closestPayload.nextPosition = closestPayload.position + closestPayload.shadingNormal * 0.001;
                        closestPayload.attenuation *= currentMaterial.albedo;
                    }
                    else {
                        pixelColor += shadeRay(&closestPayload, sceneLight, &currentMaterial);
                        closestPayload.raydepth = closestPayload.raydepth + 1;
                        closestPayload.bounce = false;
                        break;
                    }
                }
                else {
                    // if(closestPayload.raydepth == 0) {
                        var background : vec3<f32> = vec3<f32>(0.1, 0.1, 0.1); //noise(x, y), noise(2*x, 2*y), noise(4*x, 4*y));
                        // var background : vec3<f32> = vec3<f32>(0.0, 0.0, 1.0); //noise(x, y), noise(2*x, 2*y), noise(4*x, 4*y));
                        // if(ray.direction.y < 0.0){
                        //     background = vec3<f32>(1.0, 0.0, 0.0);
                        // }

                        pixelColor += vec4<f32>(closestPayload.attenuation * background, 1.0);
                    // }
                    // else {
                    //     pixelColor += vec4<f32>(closestPayload.attenuation * vec3<f32>(0.01, 0.01, 0.01), 1.0);
                    // }
                    
                    closestPayload.bounce = false;
                    break;
                }
            }
            pixelColor[3] = 1.0;
            let uv : vec2<u32> = vec2<u32>(x,y);
            textureStore(targetTexture, uv, pixelColor);
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

void PathTracingPipeline::run(WGpuDevice* device, wgpu::Queue* queue)
{
    if(!m_IsValid) return;

    // wgpu::Queue queue = device->getHandle().GetQueue();

    wgpu::ComputePassDescriptor desc{};
    desc.label = "Path Tracer pass descriptor";

    wgpu::CommandEncoder encoder = device->getHandle().CreateCommandEncoder();

    wgpu::ComputePassEncoder computePass = encoder.BeginComputePass(&desc);
    computePass.SetPipeline(m_Pipeline->getPipeline());
    computePass.SetLabel("Path Tracing Pass");
    computePass.SetBindGroup(0, m_BindGroup->get());

    computePass.DispatchWorkgroups(8, 8, 1);

    computePass.End();

    wgpu::CommandBuffer commands = encoder.Finish();

    queue->Submit(1, &commands);

    // Tímabundið
    EM_ASM({
        var elm = document.getElementById("TotalTriangleCount");
        elm.innerHTML = "# Total Triangles: N/A";
        elm = document.getElementById("UniqueObjectCount");
        elm.innerHTML = "# Objects:  N/A";
        elm = document.getElementById("UniquePartCount");
        elm.innerHTML = "# Unique Parts:  N/A";
        elm = document.getElementById("UniqueTriangleCount");
        elm.innerHTML = "# Unique Triangles:  N/A";
    });
}

WGpuTexture* PathTracingPipeline::getTargetBuffer()
{
    return m_TargetTexture;
}