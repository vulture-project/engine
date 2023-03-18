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

in vec2 uv;
out vec4 out_color;

vec2 pack2(float value)
{
    int ivalue = int(value * 256.0 * 256.0);
    int ix     = ivalue % 256;
    int iy     = ivalue / 256;
    return vec2(float(ix) / 255.0, float(iy) / 255.0);
}

void main()
{
    if (texture(u_outline_mask, uv).r >= 0.5)
    {
        out_color.rg = pack2(uv.x);
        out_color.ba = pack2(uv.y);
    }
    else
    {
        out_color = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
