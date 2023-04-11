layout(set = 1, binding = 0) uniform ViewData
{
    mat4 uView;
    mat4 uProj;
    vec3 uCameraWS;
    float uCameraNearPlane;
    float uCameraFarPlane;
};