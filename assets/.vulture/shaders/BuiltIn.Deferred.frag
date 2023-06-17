#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"
#include "include/BuiltIn.SceneData.glsl"
#include "include/BuiltIn.CascadedShadowMap.glsl"
#include "include/BuiltIn.PBR.glsl"

layout(set = 3, binding = 0) uniform sampler2D uGBuffer_Position;
layout(set = 3, binding = 1) uniform sampler2D uGBuffer_Normal;
layout(set = 3, binding = 2) uniform sampler2D uGBuffer_Albedo;
layout(set = 3, binding = 3) uniform sampler2D uGBuffer_AO_Metal_Rough;

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

void main() {
    SurfacePoint point;

    point.n             = texture(uGBuffer_Normal, texCoords).xyz;
    point.p             = texture(uGBuffer_Position, texCoords).xyz;
    point.v             = normalize(uCameraWS - point.p);

    point.surface_color = texture(uGBuffer_Albedo, texCoords).rgb;

    vec3 ao_metal_rough = texture(uGBuffer_AO_Metal_Rough, texCoords).xyz;
    point.metallic      = ao_metal_rough.y;
    point.roughness     = ao_metal_rough.z;

    point.F0            = vec3(0.04);
    point.F0            = mix(point.F0, point.surface_color, point.metallic);

    vec3 L0 = vec3(0.0);

    for (int i = 0; i < uDirectionalLightsCount; ++i) {
        LightInfo light;

        light.l = -normalize(directionalLights[i].directionWS);
        light.h = normalize(point.v + light.l);

        light.radiance = directionalLights[i].color * directionalLights[i].intensity;

        if (i == 0) {
            L0 += CalculateShadow(point.p) * CalculateLightContribution(point, light);
        } else {
            L0 += CalculateLightContribution(point, light);
        }
    }

    for (int i = 0; i < uPointLightsCount; ++i) {
        LightInfo light;

        light.l = normalize(pointLights[i].positionWS - point.p);
        light.h = normalize(point.v + light.l);

        // Attenuation
        float dist = length(pointLights[i].positionWS - point.p);
        float attenuation = 1.0 / (1.0 + (dist * dist) / (pointLights[i].range * pointLights[i].range));

        light.radiance = pointLights[i].color * pointLights[i].intensity * attenuation;

        L0 += CalculateLightContribution(point, light);
    }

    // for (int i = 0; i < uSpotLightsCount; ++i) {
    //     outColor += vec4(CalculateSpotLight(spotLights[i], bumpNormalWS, toCameraWS), 0);
    // }

//    // HDR tonemapping
//     L0 = L0 / (L0 + vec3(1.0));
////     gamma correct
//     L0 = pow(L0, vec3(1.0/2.2));

    outColor = vec4(L0, 1.0);
}