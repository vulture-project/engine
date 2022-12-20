#shader vertex
#version 330 core

layout(location = 0) in vec3 in_ms_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_ms_normal;
layout(location = 3) in vec4 in_ms_tangent; // tangent.w = +-1, it shows the handedness of TBN

out vec3 ws_position;
out vec2 uv;
out vec3 ws_normal;
out vec4 ws_tangent;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
    ws_position = (u_model * vec4(in_ms_position, 1.0)).xyz;
    uv          = in_uv;

    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    ws_normal          = normalize(normal_matrix * in_ms_normal);
    ws_tangent         = vec4(normalize(normal_matrix * in_ms_tangent.xyz), in_ms_tangent.w);

    gl_Position = u_projection * u_view * vec4(ws_position, 1.0);
}

#shader fragment
#version 330 core

#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_POINT_LIGHTS 15
#define MAX_SPOT_LIGHTS 2

struct Material
{
    vec3 color_diffuse;
    float specular;
    // float transparency;
    float normal_strength;

    // sampler2D map_ambient;
    sampler2D map_diffuse;
    // sampler2D map_specular;
    sampler2D map_normal;

    bool use_map_normal;
};

in vec3 ws_position;
in vec2 uv;
in vec3 ws_normal;
in vec4 ws_tangent;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuseSpec;

uniform int u_render_mode;

uniform vec3 u_ws_camera;

uniform Material u_material;

vec3 CalculateBumpNormal();

void main()
{
    if (texture(u_material.map_diffuse, uv).a < 0.1)
    {
        discard;
    }

    vec3 ws_bump_normal = CalculateBumpNormal();
    vec3 ws_to_camera   = normalize(u_ws_camera - ws_position);

    gPosition    = vec4(ws_position, 1);
    gNormal      = vec4(ws_bump_normal, 1);
    gDiffuseSpec = vec4(u_material.color_diffuse * texture(u_material.map_diffuse, uv).rgb, u_material.specular);
}

vec3 CalculateBumpNormal()
{
    if (!u_material.use_map_normal)
    {
        return ws_normal;
    }
    else
    {
        vec3 ws_bump_normal = normalize(ws_normal);
        
        // Gram-Schmidt proccess to make TBN orthonormal
        vec3 ws_bump_tangent = normalize(ws_tangent.xyz);
        ws_bump_tangent      = normalize(ws_bump_tangent - dot(ws_bump_normal, ws_bump_tangent) * ws_bump_normal);

        vec3 ws_bump_bitangent = normalize(cross(ws_bump_tangent, ws_bump_normal)) * ws_tangent.w;

        mat3 tbn = mat3(ws_bump_tangent, ws_bump_bitangent, ws_bump_normal);

        ws_bump_normal = tbn * (2.0 * texture(u_material.map_normal, uv).xyz - vec3(1.0));
        ws_bump_normal = normalize(ws_bump_normal);

        ws_bump_normal.xy *= u_material.normal_strength;
        ws_bump_normal = normalize(ws_bump_normal);

        return ws_bump_normal;
    }
}