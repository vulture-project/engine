#shader vertex
#version 330 core

layout(location = 0) in vec2 in_ms_position;
layout(location = 1) in vec2 in_uv;

out vec2 uv;

void main()
{
    uv          = in_uv;
    gl_Position = vec4(in_ms_position, 0.0, 1.0);
}

#shader fragment
#version 330 core
#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_POINT_LIGHTS 15
#define MAX_SPOT_LIGHTS 2

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

in vec2 uv;

out vec4 out_color;

uniform int u_render_mode;

uniform DirectionalLight u_directional_lights[MAX_DIRECTIONAL_LIGHTS];
uniform int u_directional_lights_count;

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform int u_point_lights_count;

uniform SpotLight u_spot_lights[MAX_SPOT_LIGHTS];
uniform int u_spot_lights_count;

uniform vec3 u_ws_camera;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuseSpec;

void main()
{
    vec3 ws_position  = texture(gPosition, uv).rgb;
    vec3 ws_normal    = texture(gNormal, uv).rgb;
    vec3 diffuse      = texture(gDiffuseSpec, uv).rgb;
    float specular    = texture(gDiffuseSpec, uv).a;
    vec3 ws_to_camera = normalize(u_ws_camera - ws_position);

    out_color = vec4(0, 0, 0, 1);
    for (int i = 0; i < u_directional_lights_count; ++i)
    {
        DirectionalLight light = u_directional_lights[i];
        vec3 ws_light_direction = -normalize(light.ws_direction);
        
        vec3 ambient_color = light.color_ambient * diffuse;
        vec3 diffuse_color = light.color_diffuse * diffuse * max(dot(ws_normal, ws_light_direction), 0);

        vec3 ws_halfway_direction = normalize(ws_to_camera + ws_light_direction);
        float specular_multiplier = pow(max(dot(ws_normal, ws_halfway_direction), 0.0), 16.0);
        vec3 specular_color = light.color_specular * specular_multiplier * specular;

        out_color.rgb += ambient_color + diffuse_color + specular_color;
    }

    for (int i = 0; i < u_point_lights_count; ++i)
    {
        PointLight light = u_point_lights[i];
        vec3 ws_to_light = normalize(light.ws_position - ws_position);

        vec3 ambient_color = light.color_ambient * diffuse;
        vec3 diffuse_color = light.color_diffuse * diffuse * max(dot(ws_normal, ws_to_light), 0);

        vec3 ws_halfway_direction = normalize(ws_to_camera + ws_to_light);
        float specular_multiplier = pow(max(dot(ws_normal, ws_halfway_direction), 0.0), 16.0);
        vec3 specular_color = light.color_specular * specular_multiplier * specular;

        float dist = length(light.ws_position - ws_position);
        float attenuation = 1.0 / (1.0 + dist * light.attenuation_linear + dist * dist * light.attenuation_quadratic);

        out_color.rgb += attenuation * (ambient_color + diffuse_color + specular_color);
    }

    for (int i = 0; i < u_spot_lights_count; ++i)
    {
        SpotLight light = u_spot_lights[i];
        vec3 ws_to_light = normalize(light.ws_position - ws_position);

        vec3 ambient_color = light.color_ambient * diffuse;
        vec3 diffuse_color = light.color_diffuse * diffuse * max(dot(ws_normal, ws_to_light), 0);

        vec3 ws_halfway_direction = normalize(ws_to_camera + ws_to_light);
        float specular_multiplier = pow(max(dot(ws_normal, ws_halfway_direction), 0.0), 16.0);
        vec3 specular_color = light.color_specular * specular_multiplier * specular;

        float dist = length(light.ws_position - ws_position);
        float attenuation = 1.0 / (1.0 + dist * light.attenuation_linear + dist * dist * light.attenuation_quadratic);

        float cos_to_position = dot(-ws_to_light, normalize(light.ws_direction));
        float spotlight_intensity = clamp((cos_to_position - light.outer_cone_cosine) / (light.inner_cone_cosine - light.outer_cone_cosine), 0, 1);

        out_color.rgb += spotlight_intensity * attenuation * (ambient_color + diffuse_color + specular_color);
    }

    float depth = length(ws_position - u_ws_camera) / 100;

    switch (u_render_mode)
    {
        case 0: { out_color = out_color;                                   break; }
        case 1: { out_color = vec4(ws_normal, 1.0);                        break; }
        // case 2: { out_color = texture(u_material.map_normal, uv);         break; }
        // case 3: { out_color = vec4(ws_tangent.xyz, 1.0);                  break; }
        case 4: { out_color = vec4(depth, depth, depth, 1);                break; }
    }
}