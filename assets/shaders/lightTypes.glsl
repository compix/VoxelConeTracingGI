#ifndef LIGHT_TYPE_GLSL
#define LIGHT_TYPE_GLSL

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
    float intensity;
};

struct DirectionalLightShadowDesc
{
    mat4 view;
    mat4 proj;
    float zNear;
    float zFar;
    float pcfRadius;
    int enabled;
};

#endif