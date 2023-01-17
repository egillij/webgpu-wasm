#pragma once

static const char pbrRenderCode[] = R"(

    struct VertexOutput {
        @builtin(position) position : vec4<f32>,
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>,
        @location(2) fragPosition : vec3<f32>,
    };

    struct SceneUniforms {
        viewProjection : mat4x4<f32>,
        view : mat4x4<f32>,
        cameraPosition : vec3<f32>
    };

    struct ModelUniforms {
        modelMatrix : mat4x4<f32>,
        normalMatrix : mat4x4<f32>,
    };

    @group(0) @binding(0) var<uniform> sceneUniforms : SceneUniforms;
    @group(1) @binding(0) var<uniform> modelUniforms : ModelUniforms;

    @vertex
    fn main_v(
        @location(0) positionIn : vec3<f32>,
        @location(1) normalIn : vec3<f32>,
        @location(2) uvIn : vec2<f32>
    ) -> VertexOutput {

        var output : VertexOutput;
        let fragPosition = modelUniforms.modelMatrix * vec4<f32>(positionIn, 1.0);
        output.position = sceneUniforms.viewProjection * fragPosition;
        output.fragUV = uvIn;
        output.fragNormal = (modelUniforms.normalMatrix * vec4<f32>(normalIn, 0.0)).xyz;
        output.fragPosition = fragPosition.xyz;
        return output;
    }

    
    const M_PI : f32 = 3.141592653589793;
    const M_1_PI : f32 = 0.318309886183791;

    struct FragmentOutput {
        @location(0) albedoMetallic : vec4<f32>,
        @location(1) positionRoughness : vec4<f32>,
        @location(2) normalsAo : vec4<f32>
    };

     struct MaterialParameters {
        albedo : vec3<f32>,
        metallic : f32,
        specular : vec3<f32>,
        roughness : f32,
    };

    @group(2) @binding(0) var<uniform> materialParameters : MaterialParameters;
    @group(2) @binding(1) var albedoTexture : texture_2d<f32>;
    @group(2) @binding(2) var metallicTexture : texture_2d<f32>;
    @group(2) @binding(3) var roughnessTexture : texture_2d<f32>;
    @group(2) @binding(4) var aoTexture : texture_2d<f32>;

    @group(3) @binding(0) var nearestSampler : sampler;

    @fragment
    fn main_f(
        @location(0) fragUV : vec2<f32>,
        @location(1) fragNormal : vec3<f32>,
        @location(2) fragPosition : vec3<f32>
    ) -> FragmentOutput {
        
        var N = normalize(fragNormal);
        
        let albedoSample : vec3<f32> = textureSample(albedoTexture, nearestSampler, fragUV).rgb;
        let metallic : f32 = textureSample(metallicTexture, nearestSampler, fragUV).r;
        let roughness : f32 = textureSample(roughnessTexture, nearestSampler, fragUV).r;
        let ao : f32 = textureSample(aoTexture, nearestSampler, fragUV).r;

        var output : FragmentOutput;
        output.albedoMetallic = vec4<f32>(albedoSample, metallic);
        output.positionRoughness = vec4<f32>(fragPosition, roughness);
        output.normalsAo = vec4<f32>(N, ao);

        return output;
    }
)";

static const char pbrLightingCode[] = R"(

    struct VertexOutput2 {
        @builtin(position) position : vec4<f32>,
        @location(0) fragUV : vec2<f32>,
    };

    @vertex
    fn main_v(
        @builtin(vertex_index) vertexIndex : u32,
    ) -> VertexOutput2 {

        var output : VertexOutput2;
        let xuv = f32((vertexIndex << 1) & 2);
        let yuv = f32(vertexIndex & 2);
        output.fragUV = vec2<f32>(xuv, yuv);
        output.position = vec4<f32>(output.fragUV * 2.0 + -1.0, 0.0, 1.0);

        return output;
    };

    const M_PI : f32 = 3.141592653589793;
    const M_1_PI : f32 = 0.318309886183791;

    fn fresnelSchlick(cosTheta : f32, F0 : vec3<f32>) -> vec3<f32> {
        //return F0 + (1.0 - F0) * pow(clamp(1.0-clampcosTheta, 0.0, 1.0), 5);
        return F0 + (1.0 - F0) * pow(1.0-cosTheta, 5);
    }

    fn DistributionGGX(NdotH : f32, roughness : f32) -> f32 {
        let a : f32 = roughness * roughness;
        let a2 : f32 = a * a;
        let NdotH2 : f32 = NdotH * NdotH;

        var den : f32 = NdotH2 * (a2 - 1.0) + 1.0;
        den = M_PI * den * den;

        return a2 / den;
    }

    fn GeometrySchlickGGX(NdotV : f32, roughness : f32) -> f32 {
        let r : f32 = roughness + 1.0;
        let k : f32 = r * r / 8.0;

        let den : f32 = NdotV * (1.0 - k) + k;

        return NdotV / den;
    }

    // V is vector from world position to eye
    // L is vector from world position to light
    fn GeometrySmith(NdotV : f32, NdotL : f32, roughness : f32) -> f32 {
        let ggx2 : f32 = GeometrySchlickGGX(NdotV, roughness);
        let ggx1 : f32 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    struct SceneUniforms {
        viewProjection : mat4x4<f32>,
        cameraPosition : vec3<f32>
    };

    const lDir = vec3<f32>(0.0, 0.0, -1.0);
    const lCol = vec3<f32>(1.0, 1.0, 1.0);

    @group(0) @binding(0) var<uniform> sceneUniforms : SceneUniforms;

    @group(1) @binding(0) var albedoMetallicTexture : texture_2d<f32>;
    @group(1) @binding(1) var positionRoughnessTexture : texture_2d<f32>;
    @group(1) @binding(2) var normalsAoTexture : texture_2d<f32>;

    @group(2) @binding(0) var nearestSampler : sampler;

    @fragment
    fn main_f(
        @location(0) fragUV : vec2<f32>,
    ) -> @location(0) vec4<f32> {
        
        // var N = normalize(fragNormal);

        // let R = reflect(lightDir, norm);
        var lookupuv : vec2<f32> = vec2<f32>(fragUV.x, 1.0-fragUV.y);
        let albedoMetallic : vec4<f32> = textureSample(albedoMetallicTexture, nearestSampler, lookupuv);
        let albedo = pow(albedoMetallic.rgb, vec3<f32>(2.2));
        let metallic = albedoMetallic.w;

        let positionRoughness : vec4<f32> = textureSample(positionRoughnessTexture, nearestSampler, lookupuv);
        let position : vec3<f32> = positionRoughness.xyz;
        let roughness = positionRoughness.w;

        let normalsAo : vec4<f32> = textureSample(normalsAoTexture, nearestSampler, lookupuv);
        var N = normalize(normalsAo.xyz);
        let ao = normalsAo.w;

        // View vector
        let V = normalize(position - sceneUniforms.cameraPosition);

        // Flip normal if it is facing away from the surface
        let N_dot_V = dot(N, -V);
        if(N_dot_V < 0.0)   {
            N = -N;
        }
    
        // let fragColor = vec4<f32>(albedo, 1.0);
        // return fragColor;

        var F0 : vec3<f32> = vec3<f32>(0.04);
        F0 = mix(F0, albedo, metallic);

        var Lo : vec3<f32> = vec3<f32>(0.0);

        // Lighting
        let L = lDir; // Already normalized

        let H : vec3<f32> = normalize(-V-L);

        let NdotL : f32 = max(0.0, dot(N, -L));
        let NdotV : f32 = max(0.0, dot(N, -V));
        let NdotH : f32 = max(0.0, dot(N, H));
        let HdotV : f32 = max(0.0, dot(H, -V));

        let radiance : vec3<f32> = lCol;

        let NDF : f32 = DistributionGGX(NdotH, roughness);
        let G : f32 = GeometrySmith(NdotV, NdotL, roughness);
        let F : vec3<f32> = fresnelSchlick(HdotV, F0);

        let kS : vec3<f32> = F;
        var kD : vec3<f32> = vec3<f32>(1.0) - kS;
        kD *= 1.0 - metallic;

        let num : vec3<f32> = NDF * G * F;
        let den : f32 = 4.0 * NdotV * NdotL + 0.0001;
        let specular = num / den;

        Lo = (M_1_PI * kD * albedo + specular) * radiance * NdotL;
        
        let ambient = ao * albedo;

        var color = ambient +  Lo;

        // Tone mapping
        color = color / (color + vec3(1.0));

        // Gamma correction
        let gamma : f32 = 1.0 / 2.2;
        let gammaVec : vec3<f32> = vec3<f32>(gamma, gamma, gamma);

        let fragColor = vec4<f32>(pow(color, gammaVec), 1.0);
        return fragColor;
    }
)";