#ifndef SHADOWS_GLSL
#define SHADOWS_GLSL

#extension GL_ARB_shading_language_include : enable
#include "/voxelConeTracing/settings.glsl"
#include "/lightTypes.glsl"

const int PCF_SAMPLE_COUNT = 32;
const int PCF_STEP_COUNT = 4;

const vec2 poissonDisk32[32] = vec2[](
    vec2(-0.438484, 0.6460308),
    vec2(-0.1126191, 0.4236453),
    vec2(-0.7452075, 0.4970196),
    vec2(-0.1974285, 0.832146),
    vec2(-0.4998609, 0.2145164),
    vec2(-0.2336796, -0.2300579),
    vec2(-0.7053944, -0.06415817),
    vec2(-0.5557476, -0.2988455),
    vec2(-0.2955272, 0.04999729),
    vec2(-0.8149544, 0.1824653),
    vec2(0.04275889, 0.03072814),
    vec2(0.08683829, -0.2857328),
    vec2(0.4154331, -0.07766428),
    vec2(0.3134336, 0.3748937),
    vec2(0.53735, -0.5077342),
    vec2(-0.07986151, -0.7279813),
    vec2(0.1587029, -0.6196006),
    vec2(0.8165733, -0.3503825),
    vec2(0.4893304, -0.7796087),
    vec2(0.2558164, -0.9315124),
    vec2(0.1207971, 0.750447),
    vec2(0.7669976, -0.01374529),
    vec2(0.6312151, 0.3619335),
    vec2(0.9595372, 0.2514396),
    vec2(-0.9684283, -0.2476475),
    vec2( 0.5113493, 0.7172511),
    vec2(-0.3053437, -0.5583363),
    vec2(-0.4944946, -0.7728018),
    vec2(-0.7655411, -0.5810462),
    vec2(-0.01465704, -0.9811165),
    vec2(0.3346369, 0.9181388),
    vec2(-0.2877116, -0.9329633)
);

float shadowBoxFilterPCF(sampler2D shadowMap, vec2 uv, float zReceiver, float filterRadiusUV, float depthBias)
{
    float sum = 0.0f;
	float sampleCount = PCF_SAMPLE_COUNT;
	float z = zReceiver - depthBias;
	
    // Box Filter
    float n = PCF_STEP_COUNT * 2.0 + 1.0;
    sampleCount = n * n;
    
    float stepUV = filterRadiusUV / PCF_STEP_COUNT;
    for (int x = -PCF_STEP_COUNT; x <= PCF_STEP_COUNT; ++x)
    {
        for (int y = -PCF_STEP_COUNT; y <= PCF_STEP_COUNT; ++y)
        {
            vec2 offset = vec2(x, y) * stepUV;
            
            float depth = texture(shadowMap, uv + offset, 0).r;
            float depthComparison = depth < z ? 0.0 : 1.0;
            sum += depthComparison;
        }
    }
    
    return sum / sampleCount;
}

float shadowPoissonFilterPCF(sampler2D shadowMap, vec2 uv, float zReceiver, float filterRadiusUV, float depthBias)
{
    float sum = 0.0f;
	float sampleCount = PCF_SAMPLE_COUNT;
	float z = zReceiver - depthBias;
	
    for (uint i = 0; i < PCF_SAMPLE_COUNT; ++i)
    {
        vec2 offset = (poissonDisk32[i]) * filterRadiusUV;
        
        float depth = texture(shadowMap, uv + offset).r;
        float depthComparison = depth < z ? 0.0 : 1.0;
        sum += depthComparison;
    }
    
    return sum / sampleCount;
}

float computeVisibility(vec3 posW, sampler2D shadowMap, DirectionalLightShadowDesc shadowDesc, float usePoissonFilter, float depthBias)
{
	if (shadowDesc.enabled <= 0)
		return 1.0;
		
	// Shadow mapping
	vec3 lightViewPos = (shadowDesc.view * vec4(posW, 1.0)).xyz;
	lightViewPos.z /= shadowDesc.zFar - shadowDesc.zNear;
	lightViewPos.xy = (shadowDesc.proj * vec4(lightViewPos.xy, 0.0, 1.0)).xy;
	
	// Map from [-1.0, 1.0] -> [0.0, 1.0]
	lightViewPos.xy = lightViewPos.xy * 0.5 + 0.5;
	
    if (shadowDesc.pcfRadius > 0.0)
    {
        // Higher quality (Less aliasing) - lower performance
        if (usePoissonFilter > 0.0)
            return shadowPoissonFilterPCF(shadowMap, lightViewPos.xy, lightViewPos.z, shadowDesc.pcfRadius, depthBias);
        else
            return shadowBoxFilterPCF(shadowMap, lightViewPos.xy, lightViewPos.z, shadowDesc.pcfRadius, depthBias);
    }
    else
    {
    	float z = lightViewPos.z - depthBias;
        float depth = texture(shadowMap, lightViewPos.xy, 0).r;
        float depthComparison = depth < z ? 0.0 : 1.0;
        return depthComparison;
    }
}

#endif