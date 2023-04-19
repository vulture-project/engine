#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"
#include "include/BuiltIn.SceneData.glsl"
#include "include/BuiltIn.CascadedShadowMap.glsl"

layout(set = 3, binding = 0) uniform MaterialData
{
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

struct SurfacePoint {
    vec3 n;
    vec3 p;
    vec3 v;

    vec3 surface_color;
    float metallic;
    float roughness;

    vec3 F0;
};

struct LightInfo {
    vec3 l;
    vec3 h;
    vec3 radiance;
};

float CalculateShadow();

vec3 CalculateSurfaceColorFromMap();
float CalculateMetallicFromMap();
float CalculateRoughnessFromMap();
vec3 CalculateNormalFromMap();
vec3 ConvertSrgbToLinear(vec3 value);

vec3 CalculateLightContribution(SurfacePoint point, LightInfo light);

float NDF_TR_GGX(vec3 n, vec3 h, float roughness);

float GSF_Schlick_GGX(vec3 n, vec3 v, float k);
float GSF_Smith_Schlick_GGX(vec3 n, vec3 v, vec3 l, float k);

vec3 FresnelSchlick(vec3 h, vec3 v, vec3 F0);

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
            L0 += (1.0 - CalculateShadow()) * CalculateLightContribution(point, light);
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

        // outColor += vec4(CalculatePointLight(pointLights[i], bumpNormalWS, toCameraWS), 0);
    }

    // for (int i = 0; i < uSpotLightsCount; ++i) {
    //     outColor += vec4(CalculateSpotLight(spotLights[i], bumpNormalWS, toCameraWS), 0);
    // }


    // HDR tonemapping
    // L0 = L0 / (L0 + vec3(1.0));
    // gamma correct
    // L0 = pow(L0, vec3(1.0/2.2));

    const vec3 cascadeColor[CASCADES_COUNT] = {vec3(0, 0.25, 0), vec3(0.125, 0.125, 0), vec3(0, 0, 0.25), vec3(0.25, 0, 0)};

    float clip_range = uCameraFarPlane - uCameraNearPlane;

    // Cascade index
    vec4 positionVS = uView * vec4(positionWS, 1.0);
    float depthVS    = abs(positionVS.z);

    int cascade = 0;
    for (int i = 0; i < CASCADES_COUNT; ++i) {
        if (depthVS < abs(uCameraNearPlane + clip_range * uCascadeSplits[i + 1])) {
            cascade = i;
            break;
        }
    }

    outColor = vec4(L0 + cascadeColor[cascade], 1.0);

    // outColor = vec4(vec3(texture(uCascadedShadowMap, vec3(gl_FragCoord.xy, 0)).r), 1.0);
    // outColor = vec4(vec3(1.0 - CalculateShadow()), 1.0);
    // outColor = vec4(L0, 1.0);
}

float CalculateShadow() {
    float clip_range = uCameraFarPlane - uCameraNearPlane;

    // Cascade index
    vec4 positionVS = uView * vec4(positionWS, 1.0);
    float depthVS    = abs(positionVS.z);

    int cascade = 0;
    for (int i = 0; i < CASCADES_COUNT; ++i) {
        if (depthVS < uCameraNearPlane + clip_range * uCascadeSplits[i + 1]) {
            cascade = i;
            break;
        }
    }

    // Depth in the chosen cascade space
    vec4 positionCascadeSpace = uCascadeMatrices[cascade] * vec4(positionWS, 1.0);
    positionCascadeSpace.xyz = positionCascadeSpace.xyz / positionCascadeSpace.w;
    positionCascadeSpace = positionCascadeSpace * 0.5 + 0.5;  // Convert to [0,1] range

    float depth = positionCascadeSpace.z;
    if (depth > 1.0) {
        return 0.0;
    }

    // Stored depth
    float stored_depth = texture(uCascadedShadowMap, vec3(positionCascadeSpace.xy, cascade)).r;
    // return stored_depth;

    if (depth >= stored_depth) {
        return 1.0;
    }

    return 0.0;
}

vec3 CalculateSurfaceColorFromMap() {
    if (uMaterial.useAlbedoMap != 0) {
        return ConvertSrgbToLinear(texture(uAlbedoMap, texCoords).rgb);
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

vec3 CalculateLightContribution(SurfacePoint point, LightInfo light) {
    float NDF = NDF_TR_GGX(point.n, light.h, point.roughness);

    float k = (point.roughness + 1.0) * (point.roughness + 1.0) / 8.0;
    float GSF = GSF_Smith_Schlick_GGX(point.n, point.v, light.l, k);

    vec3 F = FresnelSchlick(light.h, point.v, point.F0);

    vec3 specular_num   = NDF * GSF * F;
    float specular_denom = 4.0 * max(dot(point.n, point.v), 0.0) * max(dot(point.n, light.l), 0.0) + 0.0001;
    vec3 specular = specular_num / specular_denom;

    vec3 Ks = F;
    vec3 Kd = vec3(1.0) - Ks;
    Kd *= 1.0 - point.metallic;
    
    return (Kd * point.surface_color / PI + specular) * light.radiance * max(dot(point.n, light.l), 0.0);
}

float NDF_TR_GGX(vec3 n, vec3 h, float roughness) {
    float a  = roughness * roughness;  // Apparently looks better
    float a2 = a * a; 

    float nh = max(dot(n, h), 0.0);

    float num   = a2;
    float denom = nh * nh * (a2 - 1.0) + 1.0;
         denom = PI * denom * denom;

    return num / denom;
}

float GSF_Schlick_GGX(vec3 n, vec3 v, float k) {
    float nv = max(dot(n, v), 0.0);
    return nv / (nv * (1 - k) + k);
}

float GSF_Smith_Schlick_GGX(vec3 n, vec3 v, vec3 l, float k) {
    float shadowing   = GSF_Schlick_GGX(n, l, k);
    float obstruction = GSF_Schlick_GGX(n, v, k);

    return shadowing * obstruction;
}

vec3 FresnelSchlick(vec3 h, vec3 v, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - max(dot(h, v), 0.0), 0.0, 1.0), 5.0);
}