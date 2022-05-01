#shader vertex
#version 330 core

layout(location = 0) in vec3 msPos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 msNormal;

out vec3 wsPos;
out vec3 wsNormal;

uniform vec3 u_wsLightPos;
uniform mat4 u_ProjectionView;
uniform mat4 u_Model;

void main()
{
    wsPos = (u_Model * vec4(msPos, 1.0)).xyz;
    wsNormal = (u_Model * vec4(msPos, 1.0)).xyz;
    gl_Position = u_ProjectionView * vec4(wsPos, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
in vec3 wsPos;
in vec3 wsNormal;

uniform vec3 u_wsLightPos;
uniform vec4 u_Color;

const vec3 k_Camera = vec3(0, 0, 0);

void main()
{
    color = vec4(vec3(0.3, 0, 0.3) + (vec3(0.1, 0.9, 0.1) * max(0, dot(normalize(u_wsLightPos - wsPos), normalize(wsNormal)))), 1);
    // color = vec4(1, 0, 1, 1);
}