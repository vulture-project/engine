/************************************************************************************************
 * Constants
 ************************************************************************************************/
const float PI = 3.14159265359;

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
