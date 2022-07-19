#shader vertex
#version 330 core

layout(location = 0) in vec3 in_ms_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_ms_normal;
layout(location = 3) in vec3 in_ms_tangent;

out vec3 ws_position;
out vec2 uv;
out vec3 ws_normal;
out vec3 ws_tangent;

uniform mat4 u_projection_view;
uniform mat4 u_model;

void main()
{
    ws_position = (u_model * vec4(in_ms_position, 1.0)).xyz;
    uv          = in_uv;

    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    ws_normal     = normalize(normal_matrix * in_ms_normal);
    ws_tangent    = normalize(normal_matrix * in_ms_tangent);
    ws_tangent    = normalize(ws_tangent - dot(ws_tangent, ws_normal) * ws_normal);

    gl_Position = u_projection_view * vec4(ws_position, 1.0);
}


#shader fragment
#version 330 core

#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 2

struct Material
{
    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;
    float specular_exponent;
    // float transparency;
    float normal_strength;

    // sampler2D map_ambient;
    sampler2D map_diffuse;
    // sampler2D map_specular;
    sampler2D map_normal;
};

struct DirectionalLight
{
    vec3 ws_direction;

    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;
};

struct PointLight
{
    vec3 ws_position;

    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;

    float attenuation_linear;
    float attenuation_quadratic;
};

struct SpotLight
{
    vec3 ws_position;
    vec3 ws_direction;

    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;

    float attenuation_linear;
    float attenuation_quadratic;

    float inner_cone_cosine;
    float outer_cone_cosine;
};

in vec3 ws_position;
in vec2 uv;
in vec3 ws_normal;
in vec3 ws_tangent;

out vec4 out_color;

uniform DirectionalLight u_directional_lights[MAX_DIRECTIONAL_LIGHTS];
uniform int u_directional_lights_count;

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform int u_point_lights_count;

uniform SpotLight u_spot_lights[MAX_SPOT_LIGHTS];
uniform int u_spot_lights_count;

uniform vec3 u_ws_camera;

uniform Material u_material;

vec3 CalculateBumpNormal();
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 ws_bump_normal, vec3 ws_to_camera);
vec3 CalculatePointLight(PointLight light, vec3 ws_bump_normal, vec3 ws_to_camera);
vec3 CalculateSpotLight(SpotLight light, vec3 ws_bump_normal, vec3 ws_to_camera);

void main()
{
    vec3 ws_bump_normal = CalculateBumpNormal();
    vec3 ws_to_camera   = normalize(u_ws_camera - ws_position);

    out_color = vec4(0, 0, 0, 1);

    for (int i = 0; i < u_directional_lights_count; ++i)
    {
        out_color += vec4(CalculateDirectionalLight(u_directional_lights[i], ws_bump_normal, ws_to_camera), 0);
    }

    for (int i = 0; i < u_point_lights_count; ++i)
    {
        out_color += vec4(CalculatePointLight(u_point_lights[i], ws_bump_normal, ws_to_camera), 0);
    }

    for (int i = 0; i < u_spot_lights_count; ++i)
    {
        out_color += vec4(CalculateSpotLight(u_spot_lights[i], ws_bump_normal, ws_to_camera), 0);
    }
}

vec3 CalculateBumpNormal()
{
    // Gram-Schmidt proccess to make TBN orthonormal
    vec3 ws_bump_normal    = ws_normal;
    vec3 ws_bump_tangent   = ws_tangent;
    vec3 ws_bump_bitangent = cross(ws_bump_tangent, ws_bump_normal);

    mat3 tbn = mat3(ws_bump_tangent, ws_bump_bitangent, ws_bump_normal);

    ws_bump_normal = tbn * (2.0 * texture(u_material.map_normal, uv).xyz - vec3(1.0));
    ws_bump_normal = normalize(ws_bump_normal);

    ws_bump_normal.xy *= u_material.normal_strength;
    ws_bump_normal = normalize(ws_bump_normal);

    return ws_bump_normal;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 ws_bump_normal, vec3 ws_to_camera)
{
    vec3 texture_diffuse = texture(u_material.map_diffuse, uv).xyz;

    // Ambient
    vec3 ambient = light.color_ambient * u_material.color_ambient;

    // Diffuse
    vec3 diffuse = light.color_diffuse * u_material.color_diffuse * texture_diffuse * max(dot(ws_bump_normal, -normalize(light.ws_direction)), 0);

    // Specular
    float specular_multiplier = pow(max(dot(ws_to_camera, normalize(reflect(normalize(light.ws_direction), ws_bump_normal))), 0), u_material.specular_exponent);
    vec3 specular = light.color_specular * u_material.color_specular * 1;

    // Result
    return ambient + diffuse + specular;
}

vec3 CalculatePointLight(PointLight light, vec3 ws_bump_normal, vec3 ws_to_camera)
{
    vec3 ws_to_light = normalize(light.ws_position - ws_position);
    vec3 texture_diffuse = texture(u_material.map_diffuse, uv).xyz;

    // Ambient
    vec3 ambient = light.color_ambient * u_material.color_ambient;

    // Diffuse
    vec3 diffuse = light.color_diffuse * u_material.color_diffuse * texture_diffuse * max(dot(ws_bump_normal, ws_to_light), 0);

    // Specular
    float specular_multiplier = pow(max(dot(ws_to_camera, normalize(reflect(-ws_to_light, ws_bump_normal))), 0), u_material.specular_exponent);
    vec3 specular = light.color_specular * u_material.color_specular * 1;

    // Attenuation
    float dist = length(light.ws_position - ws_position);
    float attenuation = 1.0 / (1.0 + dist * light.attenuation_linear + dist * dist * light.attenuation_quadratic);

    // Result
    return attenuation * (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 ws_bump_normal, vec3 ws_to_camera)
{
    vec3 ws_to_light = normalize(light.ws_position - ws_position);
    vec3 texture_diffuse = texture(u_material.map_diffuse, uv).xyz;

    // Ambient
    vec3 ambient = light.color_ambient * u_material.color_ambient;

    // Diffuse
    vec3 diffuse = light.color_diffuse * u_material.color_diffuse * texture_diffuse * max(dot(ws_bump_normal, ws_to_light), 0);

    // Specular
    float specular_multiplier = pow(max(dot(ws_to_camera, normalize(reflect(-ws_to_light, ws_bump_normal))), 0), u_material.specular_exponent);
    vec3 specular = light.color_specular * u_material.color_specular * 1;

    // Attenuation
    float dist = length(light.ws_position - ws_position);
    float attenuation = 1.0 / (1.0 + dist * light.attenuation_linear + dist * dist * light.attenuation_quadratic);

    // Spotlight intensity
    float cos_to_position = dot(-ws_to_light, normalize(light.ws_direction));
    float spotlight_intensity = clamp((cos_to_position - light.outer_cone_cosine) / (light.inner_cone_cosine - light.outer_cone_cosine), 0, 1);

    // Result
    return spotlight_intensity * attenuation * (ambient + diffuse + specular);
}