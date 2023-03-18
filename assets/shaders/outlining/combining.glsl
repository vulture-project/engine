#shader vertex
#version 330 core

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

out vec2 uv;

void main()
{
    gl_Position = vec4(in_position, 0.0, 1.0);
    uv = in_uv;
}

#shader fragment
#version 330 core

uniform sampler2D u_outline_mask;
uniform sampler2D u_sample;
uniform float u_outline_width;
uniform vec3 u_outline_color;

in vec2 uv;
out vec4 out_color;

float unpack2(vec2 v)
{
    int ix     = int(round(v.x * 255.0));
    int iy     = int(round(v.y * 255.0));
    int ivalue = ix + iy * 256;
    return float(ivalue) / 256.0 / 256.0;
}

float distance_squared(vec2 first, vec2 second)
{
    return (second.x - first.x) * (second.x - first.x) + (second.y - first.y) * (second.y - first.y);
}

void main()
{
    if (texture(u_outline_mask, uv).r >= 0.5)
    {
        discard;
    }

    vec4 sample_color = texture(u_sample, uv);
    vec2 seed_uv = vec2(unpack2(sample_color.rg), unpack2(sample_color.ba));

    if (sample_color != vec4(0, 0, 0, 0) && distance_squared(uv, seed_uv) <= u_outline_width * u_outline_width)
    {
        out_color = vec4(u_outline_color, 1);
    }
    else
    {
        discard;
    }
}
