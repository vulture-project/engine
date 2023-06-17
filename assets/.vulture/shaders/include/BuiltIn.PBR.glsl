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

vec3 CalculateLightContribution(SurfacePoint point, LightInfo light);
float NDF_TR_GGX(vec3 n, vec3 h, float roughness);
float GSF_Schlick_GGX(vec3 n, vec3 v, float k);
float GSF_Smith_Schlick_GGX(vec3 n, vec3 v, vec3 l, float k);
vec3 FresnelSchlick(vec3 h, vec3 v, vec3 F0);

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