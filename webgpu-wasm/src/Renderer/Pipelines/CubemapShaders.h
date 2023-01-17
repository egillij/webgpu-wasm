#pragma once

static const char cubemapVizualizationCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) lookupDirection : vec3<f32>,
    };

    struct SceneUniforms {
        projection : mat4x4<f32>,
        viewRotation : mat4x4<f32>,
    };

    @group(0) @binding(0) var<uniform> sceneUniforms : SceneUniforms;

    @vertex
    fn main_v(
        @location(0) positionIn : vec3<f32>,
    ) -> VertexOutput {

        var output : VertexOutput;
        output.lookupDirection = positionIn;
        var pos : vec4<f32> = sceneUniforms.projection * sceneUniforms.viewRotation * vec4<f32>(positionIn, 1.0);
        output.position = pos.xyww;

        return output;
    };

    @group(1) @binding(0) var texture : texture_cube<f32>;
    @group(1) @binding(1) var nearestSampler : sampler;

    @fragment
    fn main_f(
        @location(0) lookupDirection : vec3<f32>,
    ) -> @location(0) vec4<f32> {
        
        var color : vec3<f32> = textureSample(texture, nearestSampler, lookupDirection).rgb;

        return vec4<f32>(color, 1.0);
    }
)";