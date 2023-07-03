#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"
#include "include/BuiltIn.SceneData.glsl"
#include "include/BuiltIn.CascadedShadowMap.glsl"
#include "include/BuiltIn.PBR.glsl"

layout(set = 3, binding = 0) uniform MaterialData {
    vec3 albedo_color;
    float metallic;
    float roughness;

    uint useAlbedoMap;
    uint useNormalMap;

    uint useMetallicMap;
    uint useRoughnessMap;
    uint useCombinedMetallicRoughnessMap;
} uMaterial;

layout(set = 3, binding = 1) uniform sampler2D uAlbedoMap;
layout(set = 3, binding = 2) uniform sampler2D uNormalMap;
layout(set = 3, binding = 3) uniform sampler2D uMetallicMap;
layout(set = 3, binding = 4) uniform sampler2D uRoughnessMap;
layout(set = 3, binding = 5) uniform sampler2D uCombinedMetallicRoughnessMap;

layout(location = 0) in vec3 positionWS;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in mat3 TBN;

layout(location = 0) out vec4 outColor;

vec3 CalculateSurfaceColorFromMap();
float CalculateMetallicFromMap();
float CalculateRoughnessFromMap();
vec3 CalculateNormalFromMap();
vec3 ConvertSrgbToLinear(vec3 value);

void main() {
    if (texture(uAlbedoMap, texCoords).a < 0.01) {
        discard;
    }

    SurfacePoint point;

    point.n = CalculateNormalFromMap();
    point.p = positionWS;
    point.v = normalize(uCameraWS - positionWS);

    point.surface_color = CalculateSurfaceColorFromMap();
    point.metallic      = CalculateMetallicFromMap();
    point.roughness     = CalculateRoughnessFromMap();

    point.F0 = vec3(0.04);
    point.F0 = mix(point.F0, point.surface_color, point.metallic);

    vec3 L0 = vec3(0.0);

    for (int i = 0; i < uDirectionalLightsCount; ++i) {
        LightInfo light;

        light.l = -normalize(directionalLights[i].directionWS);
        light.h = normalize(point.v + light.l);

        light.radiance = directionalLights[i].color * directionalLights[i].intensity;

        if (i == 0) {
            L0 += CalculateShadow(positionWS) * CalculateLightContribution(point, light);
            // L0 += CalculateLightContribution(point, light);
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

    // HDR exposure tonemapping
    L0 = vec3(1.0) - exp(-L0 * uCameraExposure);

    outColor = vec4(L0, 1.0);
}

vec3 CalculateSurfaceColorFromMap() {
    if (uMaterial.useAlbedoMap != 0) {
        return texture(uAlbedoMap, texCoords).rgb;
    } else {
        return uMaterial.albedo_color;
    }
}

float CalculateMetallicFromMap() {
    float metallic = 0.0;
    if (uMaterial.useCombinedMetallicRoughnessMap != 0) {
        metallic = texture(uCombinedMetallicRoughnessMap, texCoords).b;
    } else if (uMaterial.useMetallicMap != 0) {
        metallic = texture(uMetallicMap, texCoords).r;
    } else {
        metallic = uMaterial.metallic;
    }

    return clamp(metallic, 0.0001, 0.9999);
}

float CalculateRoughnessFromMap() {
    float roughness = 0.0;
    if (uMaterial.useCombinedMetallicRoughnessMap != 0) {
        roughness = texture(uCombinedMetallicRoughnessMap, texCoords).g;
    } else if (uMaterial.useRoughnessMap != 0) {
        roughness = texture(uRoughnessMap, texCoords).r;
    } else {
        roughness = uMaterial.roughness;
    }

    return max(roughness, 0.05);
}

vec3 CalculateNormalFromMap()
{
    if (uMaterial.useNormalMap == 0)
    {
        return TBN[2];
    }
    else
    {
        vec3 bumpNormalWS = 2.0 * texture(uNormalMap, texCoords).xyz - 1.0;
        bumpNormalWS = normalize(bumpNormalWS);
        bumpNormalWS = normalize(TBN * bumpNormalWS);
        return bumpNormalWS;
    }
}

vec3 ConvertSrgbToLinear(vec3 value) {
    return pow(value, vec3(2.2));
}
