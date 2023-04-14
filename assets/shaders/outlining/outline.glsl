#shader vertex
#version 330 core

layout(location = 0) in vec3 in_ms_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_ms_normal;
layout(location = 3) in vec4 in_ms_tangent; // tangent.w = +-1, it shows the handedness of TBN

uniform mat4 u_projection_view;
uniform mat4 u_model;
uniform float u_outline;

void main()
{
    vec3 ws_position   = (u_model * vec4(in_ms_position, 1.0)).xyz;
    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    vec3 ws_normal     = normalize(normal_matrix * in_ms_normal);

    // gl_Position = u_projection_view * vec4(ws_position + normalize(ws_normal) * 0.08, 1.0);

    gl_Position = u_projection_view * u_model * vec4(in_ms_position + normalize(in_ms_normal) * 0.08, 1.0);
}

#shader fragment
#version 330 core

out vec4 out_color;

void main()
{
    out_color = vec4(0.9, 0.8, 0.5, 1.0);
}
