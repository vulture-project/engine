#shader vertex
#version 330 core

layout(location = 0) in vec3 in_ms_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_ms_normal;

out vec3 ws_position;
out vec3 ws_normal;

uniform mat4 u_projection_view;
uniform mat4 u_model;

void main()
{
    ws_position = (u_model * vec4(in_ms_position, 1.0)).xyz;
    ws_normal   = (u_model * vec4(in_ms_normal, 1.0)).xyz;
    gl_Position = u_projection_view * vec4(ws_position, 1.0);
}

#shader fragment
#version 330 core

#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4

struct Material
{
    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;
    float specular_exponent;
    // float transparency;

    // sampler2D map_ambient;
    // sampler2D map_diffuse;
    // sampler2D map_specular;
    // sampler2D map_normal;
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
in vec3 ws_normal;

out vec4 color;

uniform DirectionalLight u_directional_lights[MAX_DIRECTIONAL_LIGHTS];
uniform int u_directional_lights_count;

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform int u_point_lights_count;

uniform SpotLight u_spot_lights[MAX_SPOT_LIGHTS];
uniform int u_spot_lights_count;

uniform vec3 u_ws_camera;

uniform Material u_material;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 ws_position, vec3 ws_normal, vec3 ws_to_camera);
vec3 CalculatePointLight(PointLight light, vec3 ws_position, vec3 ws_normal, vec3 ws_to_camera);
vec3 CalculateSpotLight(SpotLight light, vec3 ws_position, vec3 ws_normal, vec3 ws_to_camera);

void main()
{
    color = vec4(0, 0, 0, 1);

    for (int i = 0; i < u_directional_lights_count; ++i)
    {
        color += vec4(CalculateDirectionalLight(u_directional_lights[i], ws_position, normalize(ws_normal), normalize(u_ws_camera - ws_position)), 0);
    }

    for (int i = 0; i < u_point_lights_count; ++i)
    {
        color += vec4(CalculatePointLight(u_point_lights[i], ws_position, normalize(ws_normal), normalize(u_ws_camera - ws_position)), 0);
    }

    for (int i = 0; i < u_spot_lights_count; ++i)
    {
        color += vec4(CalculateSpotLight(u_spot_lights[i], ws_position, normalize(ws_normal), normalize(u_ws_camera - ws_position)), 0);
    }
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 ws_position, vec3 ws_normal, vec3 ws_to_camera)
{
    // Ambient
    vec3 ambient = light.color_ambient * u_material.color_ambient;

    // Diffuse
    vec3 diffuse = (light.color_diffuse * u_material.color_diffuse) * max(dot(ws_normal, -normalize(light.ws_direction)), 0);

    // Specular
    float specular_multiplier = pow(max(dot(ws_to_camera, normalize(reflect(normalize(light.ws_direction), ws_normal))), 0), u_material.specular_exponent);
    vec3 specular = light.color_specular * u_material.color_specular * 1;

    // Result
    return ambient + diffuse + specular;
}

vec3 CalculatePointLight(PointLight light, vec3 ws_position, vec3 ws_normal, vec3 ws_to_camera)
{
    vec3 ws_to_light = normalize(light.ws_position - ws_position);

    // Ambient
    vec3 ambient = light.color_ambient * u_material.color_ambient;

    // Diffuse
    vec3 diffuse = (light.color_diffuse * u_material.color_diffuse) * max(dot(ws_normal, ws_to_light), 0);

    // Specular
    float specular_multiplier = pow(max(dot(ws_to_camera, normalize(reflect(-ws_to_light, ws_normal))), 0), u_material.specular_exponent);
    vec3 specular = light.color_specular * u_material.color_specular * 1;

    // Attenuation
    float dist = length(light.ws_position - ws_position);
    float attenuation = 1.0 / (1.0 + dist * light.attenuation_linear + dist * dist * light.attenuation_quadratic);

    // Result
    return attenuation * (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 ws_position, vec3 ws_normal, vec3 ws_to_camera)
{
    vec3 ws_to_light = normalize(light.ws_position - ws_position);

    // Ambient
    vec3 ambient = light.color_ambient * u_material.color_ambient;

    // Diffuse
    vec3 diffuse = (light.color_diffuse * u_material.color_diffuse) * max(dot(ws_normal, ws_to_light), 0);

    // Specular
    float specular_multiplier = pow(max(dot(ws_to_camera, normalize(reflect(-ws_to_light, ws_normal))), 0), u_material.specular_exponent);
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