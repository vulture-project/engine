#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"
#include "include/BuiltIn.SceneData.glsl"
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

layout(location = 0) out vec3 outGBufferPosition;
layout(location = 1) out vec3 outGBufferNormal;
layout(location = 2) out vec3 outGBufferAlbedo;
layout(location = 3) out vec3 outGBufferAoMetalRough;

vec3 CalculateSurfaceColorFromMap();
float CalculateMetallicFromMap();
float CalculateRoughnessFromMap();
vec3 CalculateNormalFromMap();

void main() {
    if (texture(uAlbedoMap, texCoords).a < 0.01) {
        discard;
    }

    outGBufferPosition     = positionWS;
    outGBufferNormal       = CalculateNormalFromMap();
    outGBufferAlbedo       = CalculateSurfaceColorFromMap();
    outGBufferAoMetalRough = vec3(0, CalculateMetallicFromMap(), CalculateRoughnessFromMap());
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