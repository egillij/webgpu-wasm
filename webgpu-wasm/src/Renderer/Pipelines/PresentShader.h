#pragma once

static const char presentCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) fragUV : vec2<f32>,
    };

    @vertex
    fn main_v(
        @builtin(vertex_index) vertexIndex : u32,
    ) -> VertexOutput {

        var output : VertexOutput;
        let xuv = f32((vertexIndex << 1) & 2);
        let yuv = f32(vertexIndex & 2);
        output.fragUV = vec2<f32>(xuv, yuv);
        output.position = vec4<f32>(output.fragUV * 2.0 + -1.0, 0.0, 1.0);

        return output;
    };

    @group(0) @binding(0) var texture : texture_2d<f32>;
    @group(0) @binding(1) var nearestSampler : sampler;

    @fragment
    fn main_f(
        @location(0) fragUV : vec2<f32>,
    ) -> @location(0) vec4<f32> {
        
        var lookupuv : vec2<f32> = vec2<f32>(fragUV.x, 1.0-fragUV.y);
        var color : vec3<f32> = textureSample(texture, nearestSampler, lookupuv).rgb;

        // Tone mapping
        color = color / (color + vec3(1.0));

        // Gamma correction
        let gamma : f32 = 1.0 / 2.2;
        let gammaVec : vec3<f32> = vec3<f32>(gamma, gamma, gamma);

        let fragColor = vec4<f32>(pow(color, gammaVec), 1.0);
        return fragColor;
    }
)";