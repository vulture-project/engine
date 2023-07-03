#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"
#include "include/BuiltIn.SceneData.glsl"
#include "include/BuiltIn.CascadedShadowMap.glsl"

layout(triangles, invocations = CASCADES_COUNT) in;
layout(triangle_strip, max_vertices = 3) out;
    
void main()
{          
    for (int i = 0; i < CASCADES_COUNT; ++i)
    {
        gl_Position = uCascadeMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}  