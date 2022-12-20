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

uniform sampler2D u_sample;
uniform float u_jump_offset;
uniform vec2 u_pixel_size; // vec2(1 / screen_width, 1 / screen_height)

in vec2 uv;
out vec4 out_color;

vec2 pack2(float value)
{
    int ivalue = int(value * 256.0 * 256.0);
    int ix     = ivalue % 256;
    int iy     = ivalue / 256;
    return vec2(float(ix) / 255.0, float(iy) / 255.0);
}

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
    vec2 nearest_seed_uv = vec2(0.0, 0.0);
    bool found = false;
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            vec2 sample_uv = uv + vec2(x, y) * u_jump_offset * u_pixel_size;
            sample_uv.x = clamp(sample_uv.x, 0.0, 1.0);
            sample_uv.y = clamp(sample_uv.y, 0.0, 1.0);

            vec4 sample_color = texture(u_sample, sample_uv);
            if (sample_color != vec4(0, 0, 0, 0))
            {
                vec2 seed_uv = vec2(unpack2(sample_color.rg), unpack2(sample_color.ba));

                if (!found)
                {
                    nearest_seed_uv = seed_uv;
                    found = true;
                }
                else if (distance_squared(uv, seed_uv) < distance_squared(uv, nearest_seed_uv))
                {
                    nearest_seed_uv = seed_uv;
                } 
            }
        }
    }

    if (found)
    {
        out_color.rg = pack2(nearest_seed_uv.x);
        out_color.ba = pack2(nearest_seed_uv.y);
    }
    else
    {
        out_color = vec4(0, 0, 0, 0);
    }
}
