#shader vertex
#version 330 core

layout(location = 0) in vec3 in_ms_position;

out vec3 texture_coordinates;

uniform mat4 u_projection_view;
uniform mat4 u_model;

void main()
{
    texture_coordinates = in_ms_position;

    // Passing w as z so that the resulting z-coordinate (and consequently depth value) in NDC is always 1.0
    gl_Position = (u_projection_view * u_model * vec4(in_ms_position, 1.0)).xyww;
}

#shader fragment
#version 330 core

in vec3 texture_coordinates;
out vec4 out_color;
uniform samplerCube u_skybox;

void main()
{    
    out_color = texture(u_skybox, texture_coordinates);
}