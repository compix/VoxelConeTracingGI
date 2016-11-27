#version 430
#extension GL_ARB_shading_language_include : enable

#include "/voxelConeTracing/settings.glsl"
#include "/BRDF.glsl"
#include "/voxelConeTracing/common.glsl"
#include "/shadows/shadows.glsl"
#include "/intersection.glsl"
#include "/voxelConeTracing/conversion.glsl"

in Vertex
{
    vec2 texCoords;
} In;

#define DIRECT_LIGHTING_BIT 1
#define INDIRECT_DIFFUSE_LIGHTING_BIT 2
#define INDIRECT_SPECULAR_LIGHTING_BIT 4
#define AMBIENT_OCCLUSION_BIT 8

const float MAX_TRACE_DISTANCE = 30.0;
const float MIN_STEP_FACTOR = 0.2;
const float MIN_SPECULAR_APERTURE = 0.05;

uniform sampler2D u_diffuseTexture;
uniform sampler2D u_normalMap;
uniform sampler2D u_specularMap;
uniform sampler2D u_emissionMap;
uniform sampler2D u_depthTexture;

uniform sampler3D u_voxelRadiance;

uniform DirectionalLight u_directionalLights[MAX_DIR_LIGHT_COUNT];
uniform DirectionalLightShadowDesc u_directionalLightShadowDescs[MAX_DIR_LIGHT_COUNT];
uniform sampler2D u_shadowMaps[MAX_DIR_LIGHT_COUNT];
uniform int u_numActiveDirLights;
uniform float u_depthBias;
uniform float u_usePoissonFilter;

uniform int u_BRDFMode;
uniform mat4 u_viewProjInv;
uniform uint u_volumeDimension;
uniform vec3 u_eyePos;
uniform float u_voxelSizeL0;
uniform vec3 u_volumeCenterL0;
uniform float u_stepFactor;
uniform int u_lightingMask;
uniform float u_indirectDiffuseIntensity;
uniform float u_indirectSpecularIntensity;
uniform float u_occlusionDecay = 1.0;
uniform float u_ambientOcclusionFactor;
uniform float u_traceStartOffset;
uniform int u_visualizeMinLevelSelection = 0;
uniform bool u_useNormalMapping;

layout(location = 0) out vec4 out_color;

//#define USE_32_CONES

#ifdef USE_32_CONES
// 32 Cones for higher quality (16 on average per hemisphere)
const int DIFFUSE_CONE_COUNT = 32;
const float DIFFUSE_CONE_APERTURE = 0.628319;

const vec3 DIFFUSE_CONE_DIRECTIONS[32] = {
    vec3(0.898904, 0.435512, 0.0479745),
    vec3(0.898904, -0.435512, -0.0479745),
    vec3(0.898904, 0.0479745, -0.435512),
    vec3(0.898904, -0.0479745, 0.435512),
    vec3(-0.898904, 0.435512, -0.0479745),
    vec3(-0.898904, -0.435512, 0.0479745),
    vec3(-0.898904, 0.0479745, 0.435512),
    vec3(-0.898904, -0.0479745, -0.435512),
    vec3(0.0479745, 0.898904, 0.435512),
    vec3(-0.0479745, 0.898904, -0.435512),
    vec3(-0.435512, 0.898904, 0.0479745),
    vec3(0.435512, 0.898904, -0.0479745),
    vec3(-0.0479745, -0.898904, 0.435512),
    vec3(0.0479745, -0.898904, -0.435512),
    vec3(0.435512, -0.898904, 0.0479745),
    vec3(-0.435512, -0.898904, -0.0479745),
    vec3(0.435512, 0.0479745, 0.898904),
    vec3(-0.435512, -0.0479745, 0.898904),
    vec3(0.0479745, -0.435512, 0.898904),
    vec3(-0.0479745, 0.435512, 0.898904),
    vec3(0.435512, -0.0479745, -0.898904),
    vec3(-0.435512, 0.0479745, -0.898904),
    vec3(0.0479745, 0.435512, -0.898904),
    vec3(-0.0479745, -0.435512, -0.898904),
    vec3(0.57735, 0.57735, 0.57735),
    vec3(0.57735, 0.57735, -0.57735),
    vec3(0.57735, -0.57735, 0.57735),
    vec3(0.57735, -0.57735, -0.57735),
    vec3(-0.57735, 0.57735, 0.57735),
    vec3(-0.57735, 0.57735, -0.57735),
    vec3(-0.57735, -0.57735, 0.57735),
    vec3(-0.57735, -0.57735, -0.57735)
};
#else // 16 cones for lower quality (8 on average per hemisphere)
const int DIFFUSE_CONE_COUNT = 16;
const float DIFFUSE_CONE_APERTURE = 0.872665;

const vec3 DIFFUSE_CONE_DIRECTIONS[16] = {
    vec3(0.57735, 0.57735, 0.57735),
    vec3(0.57735, -0.57735, -0.57735),
    vec3(-0.57735, 0.57735, -0.57735),
    vec3(-0.57735, -0.57735, 0.57735),
    vec3(-0.903007, -0.182696, -0.388844),
    vec3(-0.903007, 0.182696, 0.388844),
    vec3(0.903007, -0.182696, 0.388844),
    vec3(0.903007, 0.182696, -0.388844),
    vec3(-0.388844, -0.903007, -0.182696),
    vec3(0.388844, -0.903007, 0.182696),
    vec3(0.388844, 0.903007, -0.182696),
    vec3(-0.388844, 0.903007, 0.182696),
    vec3(-0.182696, -0.388844, -0.903007),
    vec3(0.182696, 0.388844, -0.903007),
    vec3(-0.182696, 0.388844, 0.903007),
    vec3(0.182696, -0.388844, 0.903007)
};
#endif

vec3 worldPosFromDepth(float depth)
{
    vec4 p = vec4(In.texCoords, depth, 1.0);
    p.xyz = p.xyz * 2.0 - 1.0;
    p = u_viewProjInv * p;
    return p.xyz / p.w;
}

float getMinLevel(vec3 posW)
{
    float distanceToCenter = length(u_volumeCenterL0 - posW);
    float minRadius = u_voxelSizeL0 * u_volumeDimension * 0.5;
    float minLevel = log2(distanceToCenter / minRadius);  
    minLevel = max(0.0, minLevel);
    
    float radius = minRadius * exp2(ceil(minLevel));
    float f = distanceToCenter / radius;
    
    // Smoothly transition from current level to the next level
    float transitionStart = 0.5;
    float c = 1.0 / (1.0 - transitionStart);
    
    return f > transitionStart ? ceil(minLevel) + (f - transitionStart) * c : ceil(minLevel);
}

vec4 sampleClipmapTexture(sampler3D clipmapTexture, vec3 posW, int clipmapLevel, vec3 faceOffsets, vec3 weight)
{
	float voxelSize = u_voxelSizeL0 * exp2(clipmapLevel);
    float extent = voxelSize * u_volumeDimension;
	
#ifdef VOXEL_TEXTURE_WITH_BORDER
	vec3 samplePos = (fract(posW / extent) * u_volumeDimension + vec3(BORDER_WIDTH)) / (float(u_volumeDimension) + 2.0 * BORDER_WIDTH);
#else
    vec3 samplePos = fract(posW / extent);
#endif

    samplePos.y += clipmapLevel;
    samplePos.y *= CLIP_LEVEL_COUNT_INV;
    samplePos.x *= FACE_COUNT_INV;

    return clamp(texture(clipmapTexture, samplePos + vec3(faceOffsets.x, 0.0, 0.0)) * weight.x +
                 texture(clipmapTexture, samplePos + vec3(faceOffsets.y, 0.0, 0.0)) * weight.y +
                 texture(clipmapTexture, samplePos + vec3(faceOffsets.z, 0.0, 0.0)) * weight.z, 0.0, 1.0);
}

vec4 sampleClipmapLinearly(sampler3D clipmapTexture, vec3 posW, float curLevel, ivec3 faceIndices, vec3 weight)
{    
    int lowerLevel = int(floor(curLevel));
    int upperLevel = int(ceil(curLevel));
    
    vec3 faceOffsets = vec3(faceIndices) * FACE_COUNT_INV;
    
    vec4 lowSample = sampleClipmapTexture(clipmapTexture, posW, lowerLevel, faceOffsets, weight);
    
	if (lowerLevel == upperLevel)
        return lowSample;
	
    vec4 highSample = sampleClipmapTexture(clipmapTexture, posW, upperLevel, faceOffsets, weight);
	
    return mix(lowSample, highSample, fract(curLevel));
}

vec4 castCone(vec3 startPos, vec3 direction, float aperture, float maxDistance, float startLevel)
{
    // Initialize accumulated color and opacity
    vec4 dst = vec4(0.0);
    // Coefficient used in the computation of the diameter of a cone
	float coneCoefficient = 2.0 * tan(aperture * 0.5);
    
    float curLevel = startLevel;
    float voxelSize = u_voxelSizeL0 * exp2(curLevel);
    
    // Offset startPos in the direction to avoid self occlusion and reduce voxel aliasing
    startPos += direction * voxelSize * u_traceStartOffset * 0.5;

    float s = 0.0;
    float diameter = max(s * coneCoefficient, u_voxelSizeL0);

    float stepFactor = max(MIN_STEP_FACTOR, u_stepFactor);
	float occlusion = 0.0;
    
    ivec3 faceIndices = computeVoxelFaceIndices(direction); // Implementation in voxelConeTracing/common.glsl
    vec3 weight = direction * direction;
    
    float curSegmentLength = voxelSize;
    
    float minRadius = u_voxelSizeL0 * u_volumeDimension * 0.5;
    
    // Ray marching - compute occlusion and radiance in one go
    while (s < maxDistance && occlusion < 1.0)
    {
        vec3 position = startPos + direction * s;
        
        float distanceToCenter = length(u_volumeCenterL0 - position);
        float minLevel = ceil(log2(distanceToCenter / minRadius));
        
        curLevel = log2(diameter / u_voxelSizeL0);
        // The startLevel is the minimum level we start off with, minLevel is the current minLevel
        // It's important to use the max of both (and curLevel of course) because we don't want to suddenly
        // sample at a lower level than we started off with and ensure that we don't sample in a level that is too low.
        curLevel = min(max(max(startLevel, curLevel), minLevel), CLIP_LEVEL_COUNT - 1);
        
        // Retrieve radiance by accessing the 3D clipmap (voxel radiance and opacity)
        vec4 radiance = sampleClipmapLinearly(u_voxelRadiance, position, curLevel, faceIndices, weight);
		float opacity = radiance.a;

        voxelSize = u_voxelSizeL0 * exp2(curLevel);
        
        // Radiance correction
        float correctionQuotient = curSegmentLength / voxelSize;
        radiance.rgb = radiance.rgb * correctionQuotient;
		
        // Opacity correction
        opacity = clamp(1.0 - pow(1.0 - opacity, correctionQuotient), 0.0, 1.0);

        vec4 src = vec4(radiance.rgb, opacity);
		
        // Front-to-back compositing
        dst += clamp(1.0 - dst.a, 0.0, 1.0) * src;
		occlusion += (1.0 - occlusion) * opacity / (1.0 + (s + voxelSize) * u_occlusionDecay);

		float sLast = s;
        s += max(diameter, u_voxelSizeL0) * stepFactor;
        curSegmentLength = (s - sLast);
        diameter = s * coneCoefficient;
    }
    
    return clamp(vec4(dst.rgb, 1.0 - occlusion), 0.0, 1.0);
}

// Used to visualize the transition of minLevel selection
vec4 minLevelToColor(float minLevel)
{
   vec4 colors[] = {
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0),
        vec4(1.0, 1.0, 0.0, 1.0),
        vec4(0.0, 1.0, 1.0, 1.0),
        vec4(1.0, 0.0, 1.0, 1.0),
        vec4(1.0, 1.0, 1.0, 1.0)
    };
	
	vec4 minLevelColor = vec4(0.0);
    
    if (minLevel < 1)
        minLevelColor = mix(colors[0], colors[1], fract(minLevel));
    else if (minLevel < 2)
        minLevelColor = mix(colors[1], colors[2], fract(minLevel));
    else if (minLevel < 3)
        minLevelColor = mix(colors[2], colors[3], fract(minLevel));
    else if (minLevel < 4)
        minLevelColor = mix(colors[3], colors[4], fract(minLevel));
    else if (minLevel < 5)
        minLevelColor = mix(colors[4], colors[5], fract(minLevel));
    else if (minLevel < 6)
        minLevelColor = mix(colors[5], colors[6], fract(minLevel));
        
	return minLevelColor * 0.5;
}

void main() 
{
    float depth = texture2D(u_depthTexture, In.texCoords).r;
    if (depth == 1.0)
        discard;
        
    vec3 diffuse = texture(u_diffuseTexture, In.texCoords).rgb;
    vec3 normal = unpackNormal(texture(u_normalMap, In.texCoords).rgb);
    vec3 emission = texture(u_emissionMap, In.texCoords).rgb;
    bool hasEmission = any(greaterThan(emission, vec3(0.0)));  
    vec3 posW = worldPosFromDepth(depth);
    vec3 view = normalize(u_eyePos - posW);
    vec4 specColor = texture(u_specularMap, In.texCoords);
    
    float minLevel = getMinLevel(posW);
    
    // Compute indirect contribution
    vec4 indirectContribution = vec4(0.0);
    
    // Offset startPos to avoid self occlusion
    float voxelSize = u_voxelSizeL0 * exp2(minLevel);
    vec3 startPos = posW + normal * voxelSize * u_traceStartOffset;
    
    float coneTraceCount = 0.0;
    float cosSum = 0.0;
	for (int i = 0; i < DIFFUSE_CONE_COUNT; ++i)
    {
		float cosTheta = dot(normal, DIFFUSE_CONE_DIRECTIONS[i]);
        
        if (cosTheta < 0.0)
            continue;
        
        coneTraceCount += 1.0;
		indirectContribution += castCone(startPos, DIFFUSE_CONE_DIRECTIONS[i], DIFFUSE_CONE_APERTURE ,MAX_TRACE_DISTANCE, minLevel) * cosTheta;
    }

    // DIFFUSE_CONE_COUNT includes cones to integrate over a sphere - on the hemisphere there are on average ~half of these cones
	indirectContribution /= DIFFUSE_CONE_COUNT * 0.5;
    indirectContribution.a *= u_ambientOcclusionFactor;
    
	indirectContribution.rgb *= diffuse * u_indirectDiffuseIntensity;
    indirectContribution = clamp(indirectContribution, 0.0, 1.0);
    
	// Specular cone
    vec3 specularConeDirection = reflect(-view, normal);
    vec3 specularContribution = vec3(0.0);
    
    float shininess = unpackShininess(specColor.a);
    float roughness = shininessToRoughness(shininess);
    
    if (any(greaterThan(specColor.rgb, vec3(EPSILON))) && specColor.a > EPSILON)
        specularContribution = castCone(startPos, specularConeDirection, max(roughness, MIN_SPECULAR_APERTURE), MAX_TRACE_DISTANCE, minLevel).rgb * specColor.rgb * u_indirectSpecularIntensity;
    
    vec3 directContribution = vec3(0.0);
    
    if (hasEmission)
    {
        directContribution += emission;
    }
    else
    {
        for (int i = 0; i < u_numActiveDirLights; ++i)
        {
            vec3 lightDir = u_directionalLights[i].direction;
            float nDotL = max(0.0, dot(normal, -lightDir));
            vec3 halfway = normalize(view - lightDir);
            
            float visibility = 1.0;
            if (u_directionalLightShadowDescs[i].enabled != 0)
            {
                visibility = computeVisibility(posW, u_shadowMaps[i], u_directionalLightShadowDescs[i], u_usePoissonFilter, u_depthBias);
            }
            
            vec3 lightColor = u_directionalLights[i].color;
            
            if (u_BRDFMode == BLINN_PHONG_MODE_IDX)
            {
                vec3 blinnPhong = blinnPhongBRDF(lightColor, diffuse, lightColor, specColor.rgb, normal, -lightDir, halfway, shininess);
                directContribution += visibility * blinnPhong * u_directionalLights[i].intensity;
            } else if (u_BRDFMode == COOK_TORRANCE_MODE_IDX)
            {
                vec3 cook = cookTorranceBRDF(-lightDir, normal, view, halfway, roughness, specColor.rgb * 0.5);
                directContribution += visibility * (cook * lightColor * specColor.rgb + lightColor * diffuse * nDotL) * u_directionalLights[i].intensity;
            }
        }
    }
    
    directContribution = clamp(directContribution, 0.0, 1.0);
    
	out_color = vec4(0.0, 0.0, 0.0, 1.0);
    
    if ((u_lightingMask & AMBIENT_OCCLUSION_BIT) != 0)
        directContribution *= indirectContribution.a;

    if ((u_lightingMask & DIRECT_LIGHTING_BIT) != 0)
        out_color.rgb += directContribution;
        
    if ((u_lightingMask & INDIRECT_DIFFUSE_LIGHTING_BIT) != 0)
        out_color.rgb += indirectContribution.rgb;
        
    if ((u_lightingMask & INDIRECT_SPECULAR_LIGHTING_BIT) != 0)
        out_color.rgb += specularContribution;
        
    // If only ambient occlusion is selected show ambient occlusion
    if (u_lightingMask == AMBIENT_OCCLUSION_BIT)
        out_color.rgb = vec3(indirectContribution.a);
		
	if (u_visualizeMinLevelSelection > 0)
		out_color *= minLevelToColor(minLevel);
	
	out_color = clamp(out_color, 0.0, 1.0);
}
