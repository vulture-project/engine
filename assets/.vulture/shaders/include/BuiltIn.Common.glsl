/************************************************************************************************
 * Constants
 ************************************************************************************************/
const float PI = 3.14159265359;

const vec2 SCREEN_QUAD_POSITIONS[6] = {
    // Lower triangle
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),

    // Upper triangle
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
};

/************************************************************************************************
 * Structs
 ************************************************************************************************/
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
