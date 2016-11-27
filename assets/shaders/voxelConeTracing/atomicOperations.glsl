#ifndef ATOMIC_OPERATIONS_GLSL
#define ATOMIC_OPERATIONS_GLSL

#extension GL_ARB_shading_language_include : enable
#extension GL_ARB_shader_image_load_store : require

#include "/voxelConeTracing/conversion.glsl"

// Source: OpenGL Insights Chapter 22 "Octree-Based Sparse Voxelization Using the GPU Hardware Rasterizer" by Cyril Crassin and Simon Green
// Modified by adding max iterations to avoid freezes and Timeout Detection and Recovery (TDR)
void imageAtomicRGBA8Avg(layout(r32ui) volatile uimage3D image, ivec3 coords, vec4 value)
{
    value.rgb *= 255.0;                 // optimize following calculations
    uint newVal = convertVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;
	int i = 0;
	const int maxIterations = 100;

    while((curStoredVal = imageAtomicCompSwap(image, coords, prevStoredVal, newVal)) != prevStoredVal && i < maxIterations)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convertRGBA8ToVec4(curStoredVal);
        rval.rgb = (rval.rgb * rval.a); // Denormalize
        vec4 curValF = rval + value;    // Add
        curValF.rgb /= curValF.a;       // Renormalize
        newVal = convertVec4ToRGBA8(curValF);
		++i;
    }
}

#endif