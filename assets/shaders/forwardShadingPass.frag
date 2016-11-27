#version 430
#extension GL_ARB_shading_language_include : enable

#include "/BRDF.glsl"
#include "/shadows/shadows.glsl"

// Attributes
in Vertex
{
	vec3 posW;
	vec3 normalW;
	vec3 tangentW;
	vec3 bitangentW;
	vec2 uv;
	vec3 lightViewPos;
	float lightZEye;
	float clipSpaceZ;
} In;

// Out
layout(location = 0) out vec4 out_color;

uniform sampler2D u_diffuseTexture0;
uniform sampler2D u_normalMap0;
uniform sampler2D u_specularMap0;
uniform sampler2D u_opacityMap0;

uniform DirectionalLight u_directionalLights[MAX_DIR_LIGHT_COUNT];
uniform DirectionalLightShadowDesc u_directionalLightShadowDescs[MAX_DIR_LIGHT_COUNT];
uniform sampler2D u_shadowMaps[MAX_DIR_LIGHT_COUNT];
uniform int u_numActiveDirLights;
uniform float u_depthBias;
uniform float u_usePoissonFilter;

uniform int u_BRDFMode;
uniform vec4 u_color;
uniform vec3 u_specularColor;
uniform float u_hasDiffuseTexture;
uniform float u_hasNormalMap;
uniform float u_hasSpecularMap;
uniform float u_hasOpacityMap;
uniform vec3 u_eyePos;
uniform float u_shininess;
uniform float u_wireframe;

void main() 
{
    if (u_hasOpacityMap > 0.0 && texture(u_opacityMap0, In.uv).r < 0.1)
        discard;
        
    vec3 diffuseColor = vec3(1.0);
    
    if (u_hasDiffuseTexture > 0.0)
    {
        diffuseColor = texture2D(u_diffuseTexture0, In.uv).rgb;
    }

    vec3 normal = normalize(In.normalW);
    vec3 view = normalize(u_eyePos - In.posW);
    
    vec3 specColor = u_specularColor;
    
    if (u_hasSpecularMap > 0.0)
    {
        specColor = texture(u_specularMap0, In.uv).rgb;
    }

    if (u_hasNormalMap > 0.0)
    {
        vec3 normalSample = texture(u_normalMap0, In.uv).rgb;
        normalSample = 2.0 * normalSample - 1.0;
        
    	vec3 tangent = normalize(In.tangentW);
        vec3 bitangent = normalize(In.bitangentW);
    
        normal = normalSample.x * tangent + normalSample.y * bitangent + normalSample.z * normal;
        normal = normalize(normal);
    }
    
    // The conversion from the specular exponent to roughness is just a (subjectively) comparable approximation
    // TODO: Write pack/unpack functions for specular exponent
    float roughness = sqrt(2.0 / (u_shininess + 2.0));
    
    vec3 directContribution = vec3(0.0);
    for (int i = 0; i < u_numActiveDirLights; ++i)
    {
        vec3 lightDir = u_directionalLights[i].direction;
        float nDotL = max(0.0, dot(normal, -lightDir));
        vec3 halfway = normalize(view - lightDir);
        
        float visibility = 1.0;
        if (u_directionalLightShadowDescs[i].enabled != 0)
        {
            visibility = computeVisibility(In.posW, u_shadowMaps[i], u_directionalLightShadowDescs[i], u_usePoissonFilter, u_depthBias);
        }

        vec3 lightColor = u_directionalLights[i].color;
        
        if (u_BRDFMode == BLINN_PHONG_MODE_IDX)
        {
            vec3 blinnPhong = blinnPhongBRDF(lightColor, diffuseColor, lightColor, specColor, normal, -lightDir, halfway, u_shininess);
            directContribution += visibility * blinnPhong * u_directionalLights[i].intensity;
        } else if (u_BRDFMode == COOK_TORRANCE_MODE_IDX)
        {
            vec3 cook = cookTorranceBRDF(-lightDir, normal, view, halfway, roughness, specColor * 0.5);
            directContribution += visibility * (cook * lightColor * specColor + lightColor * diffuseColor * nDotL) * u_directionalLights[i].intensity;
        }
    }

    directContribution = clamp(directContribution, 0.0, 1.0);

    if (u_wireframe <= 0.0)
        out_color = vec4(directContribution, 1.0);
    else
        out_color = vec4(1.0, 0.0, 0.0, 1.0);
}
