#pragma once

static const char diffuseIrradianceConvolutionCode[] = R"(

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
        
        // let normal : vec3<f32> = normalize(lookupDirection);
        let uv = SampleSphericalMap(normalize(lookupDirection));
        var color : vec3<f32> = textureSample(texture, nearestSampler, uv).rgb;
        //TODO: do convolution
        // let color : vec3<f32> = vec3<f32>(0.3, 0.4, 0.8);
        return vec4<f32>(color, 1.0);
    }
)";