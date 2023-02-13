// Copyright 2023 Egill Ingi Jacobsen

#pragma once

static const char equirectangularToCubemapCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) lookupDirection : vec3<f32>,
    };

    struct CameraUniforms {
        projection : mat4x4<f32>,
        view : mat4x4<f32>,
    };

    @group(0) @binding(0) var<uniform> cameraUniforms : CameraUniforms;

    @vertex
    fn main_v(
        @location(0) positionIn : vec3<f32>,
        @location(1) normalIn : vec3<f32>,
        @location(2) uvIn : vec2<f32>
    ) -> VertexOutput {

        var output : VertexOutput;
        output.lookupDirection = positionIn;
        var pos : vec4<f32> = cameraUniforms.projection * cameraUniforms.view * vec4<f32>(positionIn, 1.0);
        output.position = pos;

        return output;
    };

    @group(1) @binding(0) var texture : texture_2d<f32>;
    @group(1) @binding(1) var nearestSampler : sampler;

    const invAtan : vec2<f32> = vec2<f32>(0.1591, 0.3183);
    fn SampleSphericalMap(v : vec3<f32>) -> vec2<f32>
    {
        var uv : vec2<f32> = vec2<f32>(atan2(v.z, v.x), asin(v.y));
        uv *= invAtan;
        uv += 0.5;
        return uv;
    }

    @fragment
    fn main_f(
        @location(0) lookupDirection : vec3<f32>
    ) -> @location(0) vec4<f32> {
        let uv = SampleSphericalMap(normalize(lookupDirection));
        var color : vec3<f32> = textureSample(texture, nearestSampler, uv).rgb;
        return vec4<f32>(color, 1.0);
    }
)";

static const char diffuseConvolutionCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) lookupDirection : vec3<f32>,
    };

    struct CameraUniforms {
        projection : mat4x4<f32>,
        view : mat4x4<f32>,
    };

    @group(0) @binding(0) var<uniform> cameraUniforms : CameraUniforms;

    @vertex
    fn main_v(
        @location(0) positionIn : vec3<f32>,
        @location(1) normalIn : vec3<f32>,
        @location(2) uvIn : vec2<f32>
    ) -> VertexOutput {

        var output : VertexOutput;
        output.lookupDirection = positionIn;
        var pos : vec4<f32> = cameraUniforms.projection * cameraUniforms.view * vec4<f32>(positionIn, 1.0);
        output.position = pos;

        return output;
    };

    @group(1) @binding(0) var texture : texture_2d<f32>;
    @group(1) @binding(1) var nearestSampler : sampler;

    const M_PI : f32 = 3.14159265359;
    const invAtan : vec2<f32> = vec2<f32>(0.1591, 0.3183);
    fn SampleSphericalMap(v : vec3<f32>) -> vec2<f32>
    {
        var uv : vec2<f32> = vec2<f32>(atan2(v.z, v.x), asin(v.y));
        uv *= invAtan;
        uv += 0.5;
        return uv;
    }

    fn SampleDiffuse(normal : vec3<f32>) -> vec3<f32> {
        var irradiance : vec3<f32> = vec3<f32>(0.0);  

        var up : vec3<f32> = vec3<f32>(0.0, 1.0, 0.0);
        let right : vec3<f32> = normalize(cross(up, normal));
        up = normalize(cross(normal, right));

        let sampleDelta : f32 = 0.025;
        var nrSamples : f32 = 0.0; 
        // var phi : f32 = 1.0;
        // var theta : f32 = 0.5;
        for(var phi : f32 = 0.0; phi < 2.0 * M_PI; phi += sampleDelta)
        {
            for(var theta : f32 = 0.0; theta < 0.5 * M_PI; theta += sampleDelta)
            {
                let cost : f32 = cos(theta);
                let sint : f32 = sin(theta);
                let cosp : f32 = cos(phi);
                let sinp : f32 = sin(phi);
                // spherical to cartesian (in tangent space)
                let tangentSample : vec3<f32> = vec3<f32>(sint * cosp,  sint * sinp, cost);
                // tangent space to world
                let sampleVec : vec3<f32> = normalize(vec3<f32>(tangentSample.x) * right + vec3<f32>(tangentSample.y) * up + vec3<f32>(tangentSample.z) * normal);
                let uv : vec2<f32> = SampleSphericalMap(sampleVec);
                var irr = textureSample(texture, nearestSampler, uv).rgb * cost * sint;
                irradiance += irr;
                nrSamples += 1.0;
                // let uv : vec2<f32> = SampleSphericalMap(normal);
                // irradiance += textureSample(texture, nearestSampler, uv).rgb;
                // nrSamples += 1.0;
            }
            // let uv : vec2<f32> = SampleSphericalMap(normal);
            // irradiance += textureSample(texture, nearestSampler, uv).rgb;
            // nrSamples += 1.0;
        }
        irradiance = M_PI * irradiance * (1.0 / nrSamples);
        
        return irradiance;
    }

    @fragment
    fn main_f(
        @location(0) lookupDirection : vec3<f32>
    ) -> @location(0) vec4<f32> {
        let normal : vec3<f32> = normalize(lookupDirection);
        let color : vec3<f32> = SampleDiffuse(normal);
        return vec4<f32>(color, 1.0);
    }
)";
