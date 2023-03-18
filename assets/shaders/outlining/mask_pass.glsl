#shader vertex
#version 330 core

layout(location = 0) in vec3 in_ms_position;

uniform mat4 u_projection_view;
uniform mat4 u_model;

void main()
{
    gl_Position = u_projection_view * u_model * vec4(in_ms_position, 1.0);
}

#shader fragment
#version 330 core

out vec4 out_color;

void main()
{
    out_color = vec4(1.0, 1.0, 1.0, 1.0);
}
