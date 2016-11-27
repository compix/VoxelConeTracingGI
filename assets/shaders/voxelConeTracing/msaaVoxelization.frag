#version 430
#extension GL_ARB_shading_language_include : enable
#extension GL_ARB_shader_image_load_store : require

#include "/voxelConeTracing/voxelizationFrag.glsl"

// Attributes
in Geometry
{
	vec3 posW;
    vec3 normalW;
    vec2 uv;
} In;

uniform sampler2D u_diffuseTexture0;
uniform sampler2D u_emissionMap0;
uniform sampler2D u_opacityMap0;
uniform float u_hasEmissionMap;
uniform float u_hasOpacityMap;

layout(rgba8ui) uniform uimage3D u_voxelAlbedo;

void main()
{
	if (failsPreConditions(In.posW))
		discard;
		
	if (u_hasOpacityMap > 0.0 && texture(u_opacityMap0, In.uv).r < 0.1)
        discard;

	float lod = log2(float(textureSize(u_diffuseTexture0, 0).x) / u_clipmapResolution);
    vec4 texColor = textureLod(u_diffuseTexture0, In.uv, lod);
	texColor.rgb *= texColor.a;
    texColor.a = 1.0;
	
	storeVoxelColorRGBA8(u_voxelAlbedo, In.posW, texColor);
}