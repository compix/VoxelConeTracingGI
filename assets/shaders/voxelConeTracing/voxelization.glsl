#ifndef VOXELIZATION_GLSL
#define VOXELIZATION_GLSL
#extension GL_ARB_shading_language_include : enable
#extension GL_ARB_shader_image_load_store : require

#include "/voxelConeTracing/common.glsl"
#include "/intersection.glsl"
#include "/voxelConeTracing/conversion.glsl"
#include "/voxelConeTracing/atomicOperations.glsl"
#include "/voxelConeTracing/settings.glsl"

uniform int u_clipmapLevel;
uniform int u_clipmapResolution;
uniform int u_clipmapResolutionWithBorder;

uniform vec3 u_regionMin;
uniform vec3 u_regionMax;
uniform vec3 u_prevRegionMin;
uniform vec3 u_prevRegionMax;
uniform float u_downsampleTransitionRegionSize;
uniform float u_maxExtent;
uniform float u_voxelSize;

//layout(r32ui) uniform volatile coherent uimage3D u_voxelAlbedo;

// Computation of an extended triangle in clip space based on 
// "Conservative Rasterization", GPU Gems 2 Chapter 42 by Jon Hasselgren, Tomas Akenine-Möller and Lennart Ohlsson:
// http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter42.html
void computeExtendedTriangle(vec2 halfPixelSize, vec3 triangleNormalClip, inout vec4 trianglePositionsClip[3], out vec4 triangleAABBClip)
{
	float trianglePlaneD = dot(trianglePositionsClip[0].xyz, triangleNormalClip); 
    float nSign = sign(triangleNormalClip.z);
        
    // Compute plane equations
    vec3 plane[3];
    plane[0] = cross(trianglePositionsClip[0].xyw - trianglePositionsClip[2].xyw, trianglePositionsClip[2].xyw);
    plane[1] = cross(trianglePositionsClip[1].xyw - trianglePositionsClip[0].xyw, trianglePositionsClip[0].xyw);
    plane[2] = cross(trianglePositionsClip[2].xyw - trianglePositionsClip[1].xyw, trianglePositionsClip[1].xyw);
    
    // Move the planes by the appropriate semidiagonal
    plane[0].z -= nSign * dot(halfPixelSize, abs(plane[0].xy));
    plane[1].z -= nSign * dot(halfPixelSize, abs(plane[1].xy));
    plane[2].z -= nSign * dot(halfPixelSize, abs(plane[2].xy));
	
	// Compute triangle AABB in clip space
    triangleAABBClip.xy = min(trianglePositionsClip[0].xy, min(trianglePositionsClip[1].xy, trianglePositionsClip[2].xy));
	triangleAABBClip.zw = max(trianglePositionsClip[0].xy, max(trianglePositionsClip[1].xy, trianglePositionsClip[2].xy));
    
    triangleAABBClip.xy -= halfPixelSize;
    triangleAABBClip.zw += halfPixelSize;
	
	for (int i = 0; i < 3; ++i)
    {
        // Compute intersection of the planes
        trianglePositionsClip[i].xyw = cross(plane[i], plane[(i + 1) % 3]);
        trianglePositionsClip[i].xyw /= trianglePositionsClip[i].w;
        trianglePositionsClip[i].z = -(trianglePositionsClip[i].x * triangleNormalClip.x + trianglePositionsClip[i].y * triangleNormalClip.y - trianglePlaneD) / triangleNormalClip.z;
    }
}

ivec3 computeImageCoords(vec3 posW)
{
	// Avoid floating point imprecision issues by clamping to narrowed bounds
	float c = u_voxelSize * 0.25; // Error correction constant
	posW = clamp(posW, u_regionMin + c, u_regionMax - c);
	
	vec3 clipCoords = transformPosWToClipUVW(posW, u_maxExtent);

    // The & (u_clipmapResolution - 1) (aka % u_clipmapResolution) is important here because
    // clipCoords can be in [0,1] and thus cause problems at the border (value of 1) of the physical
    // clipmap since the computed value would be 1 * u_clipmapResolution and thus out of bounds.
    // The reason is that in transformPosWToClipUVW the frac() operation is used and due to floating point
    // precision limitations the operation can return 1 instead of the mathematically correct fraction.
	ivec3 imageCoords = ivec3(clipCoords * u_clipmapResolution) & (u_clipmapResolution - 1);
    
#ifdef VOXEL_TEXTURE_WITH_BORDER
	imageCoords += ivec3(BORDER_WIDTH);
#endif

	// Target the correct clipmap level
	imageCoords.y += u_clipmapResolutionWithBorder * u_clipmapLevel;
	
	return imageCoords;
}

void storeVoxelColorAtomicRGBA8Avg6Faces(layout(r32ui) volatile uimage3D image, vec3 posW, vec4 color)
{
	ivec3 imageCoords = computeImageCoords(posW);
	
	for (int i = 0; i < 6; ++i)
		imageAtomicRGBA8Avg(image, ivec3(imageCoords + vec3(u_clipmapResolutionWithBorder * i, 0, 0)), color);
}

void storeVoxelColorR32UIRGBA8(layout(r32ui) uimage3D image, vec3 posW, vec4 color)
{
	ivec3 imageCoords = computeImageCoords(posW);
	
	for (int i = 0; i < 6; ++i)	
		imageStore(image, imageCoords + ivec3(u_clipmapResolutionWithBorder * i, 0, 0), uvec4(convertVec4ToRGBA8(color * 255)));
}

void storeVoxelColorRGBA8(layout(rgba8ui) uimage3D image, vec3 posW, vec4 color)
{
	ivec3 imageCoords = computeImageCoords(posW);
	
	for (int i = 0; i < 6; ++i)
		imageStore(image, imageCoords + ivec3(u_clipmapResolutionWithBorder * i, 0, 0), uvec4(color * 255));
}

void storeVoxelColorRGBA8(layout(rgba8) writeonly image3D image, vec3 posW, vec4 color)
{
	ivec3 imageCoords = computeImageCoords(posW);
	
	for (int i = 0; i < 6; ++i)
		imageStore(image, imageCoords + ivec3(u_clipmapResolutionWithBorder * i, 0, 0), color);
}

void storeVoxelColorAtomicRGBA8Avg(layout(r32ui) volatile uimage3D image, vec3 posW, vec4 color, ivec3 faceIndices, vec3 weight) // layout(r32ui) volatile coherent 
{
	ivec3 imageCoords = computeImageCoords(posW);
	
	imageAtomicRGBA8Avg(image, imageCoords + ivec3(faceIndices.x * u_clipmapResolutionWithBorder, 0, 0), vec4(color.rgb * weight.x, 1.0));
	imageAtomicRGBA8Avg(image, imageCoords + ivec3(faceIndices.y * u_clipmapResolutionWithBorder, 0, 0), vec4(color.rgb * weight.y, 1.0));
	imageAtomicRGBA8Avg(image, imageCoords + ivec3(faceIndices.z * u_clipmapResolutionWithBorder, 0, 0), vec4(color.rgb * weight.z, 1.0));
}

//void storeVoxelColorRGBA8(uimage3D image, vec3 posW, vec4 color) // layout(rgba8ui) volatile coherent 
//{
//	ivec3 imageCoords = computeImageCoords(posW);
//	
//	for (int i = 0; i < 6; ++i)
//		imageStore(image, imageCoords + ivec3(u_clipmapResolutionWithBorder * i, 0, 0), uvec4(color * 255));
//}

#endif