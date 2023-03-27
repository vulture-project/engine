#version 450

struct DirectionalLight
{
    // Specification
    vec3 color;
    float intensity;
    // Specification

    vec3 directionWS;
};

struct PointLight
{
    // Specification
    vec3 color;
    float intensity;
    float range;
    // Specification

    vec3 positionWS;
};

struct SpotLight
{
    // Specification
    vec3 color;
    float intensity;
    float range;
    float innerCone;  // TODO: (tralf-strues) precompute cosine value
    float outerCone;  // TODO: (tralf-strues) precompute cosine value
    // Specification

    vec3 positionWS;
    vec3 directionWS;
};

/* Descriptor Sets */
layout(set = 0, binding = 0) uniform FrameData
{
    float uTime;
};

layout(set = 1, binding = 0) uniform ViewData
{
    mat4 uView;
    mat4 uProj;
    vec3 uCameraWS;
};

layout(set = 2, binding = 0) uniform LightEnvironment
{
  int uDirectionalLightsCount;
  int uPointLightsCount;
  int uSpotLightsCount;
};

layout(std140, set = 2, binding = 1) readonly buffer DirectionalLightsData
{
    DirectionalLight directionalLights[];
};

layout(std140, set = 2, binding = 2) readonly buffer PointLightsData
{
    PointLight pointLights[];
};

layout(std140, set = 2, binding = 3) readonly buffer SpotLightsData
{
    SpotLight spotLights[];
};

layout(set = 3, binding = 0) uniform MaterialData
{
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float specularExponent;
    float normalStrength;
    uint useNormalMap;
} uMaterial;

layout(set = 3, binding = 1) uniform sampler2D uDiffuseMap;
layout(set = 3, binding = 2) uniform sampler2D uNormalMap;

layout(location = 0) in vec3 positionWS;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normalWS;
layout(location = 3) in vec4 tangentWS;

layout(location = 0) out vec4 outColor;

vec3 CalculateBumpNormal();
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 bumpNormalWS, vec3 toCameraWS);
// vec3 CalculatePointLight(PointLight light, vec3 bumpNormalWS, vec3 toCameraWS);
// vec3 CalculateSpotLight(SpotLight light, vec3 bumpNormalWS, vec3 toCameraWS);

void main()
{
    if (texture(uDiffuseMap, texCoords).a < 0.01)
    {
        discard;
    }

    vec3 bumpNormalWS = CalculateBumpNormal();
    vec3 toCameraWS   = normalize(uCameraWS - positionWS);

    outColor = texture(uDiffuseMap, texCoords);

    // outColor = vec4(0, 0, 0, 0);

    // for (int i = 0; i < uDirectionalLightsCount; ++i)
    // {
    //     outColor += vec4(CalculateDirectionalLight(directionalLights[i], bumpNormalWS, toCameraWS), 0);
    // }

    // for (int i = 0; i < uPointLightsCount; ++i)
    // {
    //     outColor += vec4(CalculatePointLight(pointLights[i], bumpNormalWS, toCameraWS), 0);
    // }

    // for (int i = 0; i < uSpotLightsCount; ++i)
    // {
    //     outColor += vec4(CalculateSpotLight(spotLights[i], bumpNormalWS, toCameraWS), 0);
    // }
}

vec3 CalculateBumpNormal()
{
    if (uMaterial.useNormalMap == 0)
    {
        return normalWS;
    }
    else
    {
        vec3 bumpNormalWS    = normalize(normalWS);
        vec3 bumpTangentWS   = normalize(tangentWS.xyz);
        vec3 bumpBitangentWS = normalize(cross(bumpTangentWS, bumpNormalWS));
        mat3 tbn = mat3(bumpTangentWS, bumpBitangentWS, bumpNormalWS);

        bumpNormalWS = tbn * (2.0 * texture(uNormalMap, texCoords).xyz - vec3(1.0));
        bumpNormalWS = normalize(bumpNormalWS);

        bumpNormalWS.xy *= uMaterial.normalStrength;
        bumpNormalWS = normalize(bumpNormalWS);

        return bumpNormalWS;



        // vec3 bumpNormalWS = normalize(normalWS);
        
        // // Gram-Schmidt proccess to make TBN orthonormal
        // vec3 bumpTangentWS = normalize(tangentWS.xyz);
        // bumpTangentWS      = normalize(bumpTangentWS - dot(bumpNormalWS, bumpTangentWS) * bumpNormalWS);

        // vec3 bumpBitangentWS = normalize(cross(bumpTangentWS, bumpNormalWS)) * tangentWS.w;

        // mat3 tbn = mat3(bumpTangentWS, bumpBitangentWS, bumpNormalWS);

        // bumpNormalWS = tbn * (2.0 * texture(uNormalMap, texCoords).xyz - vec3(1.0));
        // bumpNormalWS = normalize(bumpNormalWS);

        // bumpNormalWS.xy *= uMaterial.normalStrength;
        // bumpNormalWS = normalize(bumpNormalWS);

        // return bumpNormalWS;
    }
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 bumpNormalWS, vec3 toCameraWS)
{
    vec3 textureDiffuse = texture(uDiffuseMap, texCoords).xyz;

    vec3 light_color = light.color * light.intensity;

    // Ambient
    // vec3 ambient = uMaterial.ambientColor;

    // Diffuse
    vec3 diffuse = light_color * uMaterial.diffuseColor * textureDiffuse * max(dot(bumpNormalWS, -normalize(light.directionWS)), 0);

    // Specular
    float specularMultiplier = pow(max(dot(toCameraWS, normalize(reflect(normalize(light.directionWS), bumpNormalWS))), 0), uMaterial.specularExponent);
    vec3 specular = light_color * uMaterial.specularColor * 1;

    // Result
    // return ambient + diffuse + specular;
    return diffuse + specular;
}

// vec3 CalculatePointLight(PointLight light, vec3 bumpNormalWS, vec3 toCameraWS)
// {
//     vec3 toLightWS = normalize(light.positionWS - positionWS);
//     vec3 textureDiffuse = texture(uDiffuseMap, texCoords).xyz;

//     // Ambient
//     vec3 ambient = light.ambientColor * uMaterial.ambientColor;

//     // Diffuse
//     vec3 diffuse = light.diffuseColor * uMaterial.diffuseColor * textureDiffuse * max(dot(bumpNormalWS, toLightWS), 0);

//     // Specular
//     float specularMultiplier = pow(max(dot(toCameraWS, normalize(reflect(-toLightWS, bumpNormalWS))), 0), uMaterial.specularExponent);
//     vec3 specular = light.specularColor * uMaterial.specularColor * 1;

//     // Attenuation
//     float dist = length(light.positionWS - positionWS);
//     float attenuation = 1.0 / (1.0 + dist * light.attenuationLinear + dist * dist * light.attenuationQuadratic);

//     // Result
//     return attenuation * (ambient + diffuse + specular);
// }

// vec3 CalculateSpotLight(SpotLight light, vec3 bumpNormalWS, vec3 toCameraWS)
// {
//     vec3 toLightWS = normalize(light.positionWS - positionWS);
//     vec3 textureDiffuse = texture(uDiffuseMap, texCoords).xyz;

//     // Ambient
//     vec3 ambient = light.ambientColor * uMaterial.ambientColor;

//     // Diffuse
//     vec3 diffuse = light.diffuseColor * uMaterial.diffuseColor * textureDiffuse * max(dot(bumpNormalWS, toLightWS), 0);

//     // Specular
//     float specularMultiplier = pow(max(dot(toCameraWS, normalize(reflect(-toLightWS, bumpNormalWS))), 0), uMaterial.specularExponent);
//     vec3 specular = light.specularColor * uMaterial.specularColor * 1;

//     // Attenuation
//     float dist = length(light.positionWS - positionWS);
//     float attenuation = 1.0 / (1.0 + dist * light.attenuationLinear + dist * dist * light.attenuationQuadratic);

//     // Spotlight intensity
//     float cosToPosition = dot(-toLightWS, normalize(light.directionWS));
//     float spotlightIntensity = clamp((cosToPosition - light.outerConeCosine) / (light.innerConeCosine - light.outerConeCosine), 0, 1);

//     // Result
//     return spotlightIntensity * attenuation * (ambient + diffuse + specular);
// }