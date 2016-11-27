#version 430
#extension GL_ARB_shading_language_include : enable
#extension GL_ARB_shader_image_load_store : require

#define CONSERVATIVE_VOXELIZATION

#include "/voxelConeTracing/voxelizationFrag.glsl"

in Geometry
{
    vec3 normalW;
    vec2 uv;
} In;

//uniform sampler2D u_diffuseTexture0;
//uniform float u_hasDiffuseTexture;

uniform sampler2D u_opacityMap0;
uniform float u_hasOpacityMap;
uniform vec4 u_color;

uniform layout(rgba8) writeonly image3D u_voxelOpacity;
//uniform layout(r32ui) uimage3D u_voxelOpacity;

void main() 
{
	if (cvFailsPreConditions())
		discard;
	
	if (u_hasOpacityMap > 0.0 && texture(u_opacityMap0, In.uv).r < 0.1)
        discard;
        
    // Commenting both out fixes the problem of missing voxelization
    if (isOutsideVoxelizationRegion(in_cvFrag.posW))
        discard;
      
    if (!cvIntersectsTriangle(in_cvFrag.posW))
        discard;
    
    //float opacity = u_color.a;
    
    //if (u_hasDiffuseTexture > 0.0)
    //{
    //	float lod = log2(float(textureSize(u_diffuseTexture0, 0).x) / u_clipmapResolution);
    //    vec4 texColor = textureLod(u_diffuseTexture0, In.uv, lod);
    //    float opacity = texColor.a;
    //    //texColor.rgb *= texColor.a;
    //    //texColor.a = 1.0;
    //    ivec3 imageCoords = computeImageCoords(in_cvFrag.posW);
    //    vec3 normal = normalize(In.normalW);
    //    ivec3 faceIndices = computeVoxelFaceIndices(-normal);
    //    vec3 weight = normal * normal;
    //
    //    storeVoxelColorAtomicRGBA8Avg(u_voxelOpacity, in_cvFrag.posW, texColor, faceIndices, weight);
    //    //imageAtomicRGBA8Avg(u_voxelOpacity, imageCoords + ivec3(in_cvFrag.faceIdx * u_clipmapResolutionWithBorder, 0, 0), texColor);
    //}

    
    ivec3 imageCoords = computeImageCoords(in_cvFrag.posW);

    for (int i = 0; i < 6; ++i)
    {
        // Currently not supporting alpha blending so just make it fully opaque
        imageStore(u_voxelOpacity, imageCoords, vec4(1.0));
        imageCoords.x += u_clipmapResolutionWithBorder;
    }
}
