#ifndef VOXELIZATION_FRAG_GLSL
#define VOXELIZATION_FRAG_GLSL
#extension GL_ARB_shading_language_include : enable
#extension GL_ARB_shader_image_load_store : require

#include "/voxelConeTracing/voxelization.glsl"
#include "/voxelConeTracing/common.glsl"
#include "/intersection.glsl"
#include "/voxelConeTracing/conversion.glsl"
#include "/voxelConeTracing/atomicOperations.glsl"
#include "/voxelConeTracing/settings.glsl"

bool isInsideDownsampleRegion(vec3 posW)
{
	return u_clipmapLevel > 0 && all(greaterThanEqual(posW, u_prevRegionMin + vec3(u_downsampleTransitionRegionSize))) && 
						         all(lessThanEqual(posW, u_prevRegionMax - vec3(u_downsampleTransitionRegionSize)));
}

bool isOutsideVoxelizationRegion(vec3 posW)
{
	return any(lessThan(posW, u_regionMin)) || any(greaterThan(posW, u_regionMax));
}

#ifdef CONSERVATIVE_VOXELIZATION
in ConservativeVoxelizationFragmentInput
{
	vec3 posW;
    vec3 posClip;
    flat vec4 triangleAABB;
	flat vec3[3] trianglePosW;
	flat int faceIdx;
} in_cvFrag;

bool cvIntersectsTriangle(vec3 posW)
{
    AABBox3D b;
	b.minPos = floor(posW / u_voxelSize) * u_voxelSize;
	b.maxPos = b.minPos + vec3(u_voxelSize);
	
	return aabbIntersectsTriangle(b, in_cvFrag.trianglePosW[0], in_cvFrag.trianglePosW[1], in_cvFrag.trianglePosW[2]);
}

bool cvIsValidVoxelizationCandidate(vec3 posW)
{
	if(isInsideDownsampleRegion(posW) || isOutsideVoxelizationRegion(posW))
		return false;
		
    return cvIntersectsTriangle(posW);
}

void cvStoreVoxelColor(layout(rgba8) writeonly image3D image, vec4 color)
{
	// There are potentially 3 voxels in the depth direction that can overlap the triangle
	vec3 posW0 = in_cvFrag.posW;
	vec3 posW1 = in_cvFrag.posW - u_voxelSize * MAIN_AXES[gl_ViewportIndex];
	vec3 posW2 = in_cvFrag.posW + u_voxelSize * MAIN_AXES[gl_ViewportIndex];

	if (!isOutsideVoxelizationRegion(posW0) && cvIntersectsTriangle(posW0))
		storeVoxelColorRGBA8(image, posW0, color);

	if (!isOutsideVoxelizationRegion(posW1) && cvIntersectsTriangle(posW1))
		storeVoxelColorRGBA8(image, posW1, color);

	if (!isOutsideVoxelizationRegion(posW2) && cvIntersectsTriangle(posW2))
		storeVoxelColorRGBA8(image, posW2, color);
}

void cvStoreVoxelColor(layout(r32ui) uimage3D image, vec4 color)
{
	// There are potentially 3 voxels in the depth direction that can overlap the triangle
	vec3 posW0 = in_cvFrag.posW;
	vec3 posW1 = in_cvFrag.posW - u_voxelSize * MAIN_AXES[gl_ViewportIndex];
	vec3 posW2 = in_cvFrag.posW + u_voxelSize * MAIN_AXES[gl_ViewportIndex];

	if (cvIsValidVoxelizationCandidate(posW0))
		storeVoxelColorR32UIRGBA8(image, posW0, color);

	if (cvIsValidVoxelizationCandidate(posW1))
		storeVoxelColorR32UIRGBA8(image, posW1, color);

	if (cvIsValidVoxelizationCandidate(posW2))
		storeVoxelColorR32UIRGBA8(image, posW2, color);
}

void cvStoreVoxelColorAvg(layout(r32ui) volatile uimage3D image, vec4 color, ivec3 faceIndices, vec3 weight)
{
	// There are potentially 3 voxels in the depth direction that can overlap the triangle
	vec3 posW0 = in_cvFrag.posW;
	vec3 posW1 = in_cvFrag.posW - u_voxelSize * MAIN_AXES[gl_ViewportIndex];
	vec3 posW2 = in_cvFrag.posW + u_voxelSize * MAIN_AXES[gl_ViewportIndex];

	if (cvIsValidVoxelizationCandidate(posW0))
		storeVoxelColorAtomicRGBA8Avg(image, posW0, color, faceIndices, weight);

	if (cvIsValidVoxelizationCandidate(posW1))
		storeVoxelColorAtomicRGBA8Avg(image, posW1, color, faceIndices, weight);

	if (cvIsValidVoxelizationCandidate(posW2))
		storeVoxelColorAtomicRGBA8Avg(image, posW2, color, faceIndices, weight);
}

void cvStoreVoxelColorAvg(layout(r32ui) volatile uimage3D image, vec4 color)
{
	// There are potentially 3 voxels in the depth direction that can overlap the triangle
	vec3 posW0 = in_cvFrag.posW;
	vec3 posW1 = in_cvFrag.posW - u_voxelSize * MAIN_AXES[gl_ViewportIndex];
	vec3 posW2 = in_cvFrag.posW + u_voxelSize * MAIN_AXES[gl_ViewportIndex];

	if (cvIsValidVoxelizationCandidate(posW0))
		storeVoxelColorAtomicRGBA8Avg6Faces(image, posW0, color);

	if (cvIsValidVoxelizationCandidate(posW1))
		storeVoxelColorAtomicRGBA8Avg6Faces(image, posW1, color);

	if (cvIsValidVoxelizationCandidate(posW2))
		storeVoxelColorAtomicRGBA8Avg6Faces(image, posW2, color);
}

bool cvFailsPreConditions()
{
    return (in_cvFrag.posClip.x < in_cvFrag.triangleAABB.x || in_cvFrag.posClip.y < in_cvFrag.triangleAABB.y || 
		    in_cvFrag.posClip.x > in_cvFrag.triangleAABB.z || in_cvFrag.posClip.y > in_cvFrag.triangleAABB.w);
}

#endif // CONSERVATIVE_VOXELIZATION

bool failsPreConditions(vec3 posW)
{
	return isInsideDownsampleRegion(posW) || isOutsideVoxelizationRegion(posW);
}

#endif // VOXELIZATION_FRAG_GLSL