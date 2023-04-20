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
};