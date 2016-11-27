#ifndef CONVERSION_GLSL
#define CONVERSION_GLSL

vec4 convertRGBA8ToVec4(uint val)
{
    return vec4(float((val & 0x000000FF)), 
                float((val & 0x0000FF00) >> 8U), 
                float((val & 0x00FF0000) >> 16U), 
                float((val & 0xFF000000) >> 24U));
}

uint convertVec4ToRGBA8(vec4 val)
{
    return (uint(val.w) & 0x000000FF) << 24U | 
           (uint(val.z) & 0x000000FF) << 16U | 
           (uint(val.y) & 0x000000FF) << 8U  | 
           (uint(val.x) & 0x000000FF);
}

vec3 transformPosWToClipUVW(vec3 posW, float maxExtent)
{
	return fract(posW / maxExtent);
}

vec3 transformPosWToClipUVW(vec3 posW, vec3 extent)
{
	return fract(posW / extent);
}

vec2 transformPosWToClipUV(vec2 posW, float maxExtent)
{
	return fract(posW / maxExtent);
}

vec2 transformPosWToClipUV(vec2 posW, vec2 extent)
{
	return fract(posW / extent);
}

float packShininess(float shininess)
{
    return shininess / 255.0; // assumed 255.0 to be the max possible shininess value
}

float unpackShininess(float shininess)
{
    return shininess * 255.0;
}

float shininessToRoughness(float shininess)
{
    // The conversion from the specular exponent (shininess) to roughness is just a (subjective) approximation
    return sqrt(2.0 / (shininess + 2.0));
}

vec3 unpackNormal(vec3 packedNormal)
{
    return packedNormal * 2.0 - 1.0;
}

vec3 packNormal(vec3 normal)
{
    return normal * 0.5 + 0.5;
}

#endif