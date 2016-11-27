#version 430
#extension GL_ARB_shading_language_include : enable
#extension GL_ARB_shader_image_load_store : require

#define CONSERVATIVE_VOXELIZATION

#include "/voxelConeTracing/voxelizationFrag.glsl"
#include "/voxelConeTracing/common.glsl"
#include "/shadows/shadows.glsl"

in Geometry
{
    vec3 normalW;
    vec2 uv;
} In;

uniform DirectionalLight u_directionalLights[MAX_DIR_LIGHT_COUNT];
uniform DirectionalLightShadowDesc u_directionalLightShadowDescs[MAX_DIR_LIGHT_COUNT];
uniform sampler2D u_shadowMaps[MAX_DIR_LIGHT_COUNT];
uniform int u_numActiveDirLights;
uniform float u_depthBias;
uniform float u_usePoissonFilter;

uniform sampler2D u_diffuseTexture0;
uniform sampler2D u_emissionMap0;
uniform sampler2D u_opacityMap0;
uniform float u_hasEmissionMap;
uniform float u_hasOpacityMap;
uniform float u_hasDiffuseTexture;
uniform vec4 u_color;
uniform vec3 u_emissionColor;

uniform layout(r32ui) volatile uimage3D u_voxelRadiance;

void main() 
{
	if (cvFailsPreConditions())
		discard;
	
	if (u_hasOpacityMap > 0.0 && texture(u_opacityMap0, In.uv).r < 0.1)
        discard;
	
    if (isOutsideVoxelizationRegion(in_cvFrag.posW) || isInsideDownsampleRegion(in_cvFrag.posW))
        discard;
      
    if (!cvIntersectsTriangle(in_cvFrag.posW))
        discard;
        
	float lod;
	
    if (any(greaterThan(u_emissionColor, vec3(0.0))))
    {
        vec4 emission = vec4(u_emissionColor, 1.0);
        
    	if (u_hasEmissionMap > 0.0)
        {
            lod = log2(float(textureSize(u_emissionMap0, 0).x) / u_clipmapResolution);
            emission.rgb += textureLod(u_emissionMap0, In.uv, lod).rgb;
        }
        
        emission.rgb = clamp(emission.rgb, 0.0, 1.0);
        storeVoxelColorAtomicRGBA8Avg6Faces(u_voxelRadiance, in_cvFrag.posW, emission);
    }
	else
	{
        vec4 color = u_color;
        
        if (u_hasDiffuseTexture > 0.0)
        {
            lod = log2(float(textureSize(u_diffuseTexture0, 0).x) / u_clipmapResolution);
            color = textureLod(u_diffuseTexture0, In.uv, lod);
        }
		
		vec3 normal = normalize(In.normalW);
		
        vec3 lightContribution = vec3(0.0);
        for (int i = 0; i < u_numActiveDirLights; ++i)
        {
            float nDotL = max(0.0, dot(normal, -u_directionalLights[i].direction));
            
            float visibility = 1.0;
            if (u_directionalLightShadowDescs[i].enabled != 0)
            {
                visibility = computeVisibility(in_cvFrag.posW, u_shadowMaps[i], u_directionalLightShadowDescs[i], u_usePoissonFilter, u_depthBias);
            }
            
            lightContribution += nDotL * visibility * u_directionalLights[i].color * u_directionalLights[i].intensity;
        }
        
        if (all(equal(lightContribution, vec3(0.0))))
            discard;
        
		vec3 radiance = lightContribution * color.rgb * color.a;
        radiance = clamp(radiance, 0.0, 1.0);
		
		ivec3 faceIndices = computeVoxelFaceIndices(-normal);
        storeVoxelColorAtomicRGBA8Avg(u_voxelRadiance, in_cvFrag.posW, vec4(radiance, 1.0), faceIndices, abs(normal));
	}
}
