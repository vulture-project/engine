const int CASCADES_COUNT = 3;

layout(set = 4, binding = 0) uniform sampler2DArray uCascadedShadowMap;
layout(set = 4, binding = 1) uniform CSMData {
  mat4 uCascadeMatrices[CASCADES_COUNT];

  /* Ratio of the clip range for the camera.
   * 
   * uCascadeSplits[i]     - near plane ratio for the ith cascade
   * uCascadeSplits[i + 1] - far  plane ratio for the ith cascade
   * 
   * In particular,
   * uCascadeSplits[0]              = 0.0
   * uCascadeSplits[CASCADES_COUNT] = 1.0
   */
  float uCascadeSplits[CASCADES_COUNT + 1];

  vec3 uShadowColor;
  uint uSoftShadows;
  float uBias;
};

int GetCascade(vec3 positionWS) {
    float clip_range = uCameraFarPlane - uCameraNearPlane;

    vec4 positionVS = uView * vec4(positionWS, 1.0);
    float depthVS = abs(positionVS.z);

    int cascade = 0;
    for (int i = 0; i < CASCADES_COUNT; ++i) {
        if (depthVS < uCameraNearPlane + clip_range * uCascadeSplits[i + 1]) {
            cascade = i;
            break;
        }
    }

    return cascade;
}

vec3 CalculateShadow(vec3 positionWS) {
    int cascade = GetCascade(positionWS);

    // Depth in the chosen cascade space
    vec4 positionCascadeSpace = uCascadeMatrices[cascade] * vec4(positionWS, 1.0);
    positionCascadeSpace.xyz = positionCascadeSpace.xyz / positionCascadeSpace.w;
    positionCascadeSpace.xy = positionCascadeSpace.xy * 0.5 + 0.5;  // Convert to [0,1] range

    float depth = positionCascadeSpace.z;
    if (depth > 1.0) {
        return vec3(1.0);
    }

    vec3 shadow = vec3(0.0);
    if (uSoftShadows == 1) {
        vec2 texelSize = 1.0 / textureSize(uCascadedShadowMap, 0).xy;
        vec2 baseTexel = vec2(positionCascadeSpace.x, 1.0 - positionCascadeSpace.y);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(uCascadedShadowMap, vec3(baseTexel + vec2(x, y) * texelSize, cascade)).r; 
                shadow += (depth - uBias > pcfDepth) ? uShadowColor : vec3(1.0);
            }    
        }
        shadow /= 9.0;
    } else {
        float storedDepth = texture(uCascadedShadowMap, vec3(positionCascadeSpace.x, 1.0 - positionCascadeSpace.y, cascade)).r;
        shadow = (depth - uBias > storedDepth) ? uShadowColor : vec3(1.0);
    }

    return shadow;
}

vec3 DebugCascadeColor(vec3 positionWS) {
    const vec3 CASCADE_COLOR3[3] = {vec3(0, 0.25, 0), vec3(0.125, 0.125, 0), vec3(0, 0, 0.25)};
    const vec3 CASCADE_COLOR4[4] = {vec3(0, 0.25, 0), vec3(0.125, 0.125, 0), vec3(0, 0, 0.25), vec3(0.25, 0, 0)};

    int cascade = GetCascade(positionWS);

    if (CASCADES_COUNT == 3) {
        return CASCADE_COLOR3[cascade];
    } else if (CASCADES_COUNT == 4) {
        return CASCADE_COLOR4[cascade];
    }

    return vec3(1.0);
}
