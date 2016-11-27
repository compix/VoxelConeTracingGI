#version 430
#extension GL_ARB_shading_language_include : enable

#include "/voxelConeTracing/conversion.glsl"

in Vertex
{
    vec3 normalW;
    vec3 tangentW;
    vec3 bitangentW;
    vec2 uv;
} In;

#define OPACITY_THRESHOLD 0.1

uniform sampler2D u_diffuseTexture0;
uniform sampler2D u_normalMap0;
uniform sampler2D u_specularMap0;
uniform sampler2D u_opacityMap0;
uniform sampler2D u_emissionMap0;

uniform float u_hasDiffuseTexture;
uniform float u_hasNormalMap;
uniform float u_hasSpecularMap;
uniform float u_hasOpacityMap;
uniform float u_hasEmissionMap;
uniform float u_shininess;
uniform vec3 u_emissionColor;
uniform vec3 u_specularColor;

uniform vec4 u_color;

layout (location = 0) out vec3 out_diffuse;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec4 out_specular;
layout (location = 3) out vec3 out_emission;

void main() 
{
    if (u_hasOpacityMap > 0.0 && texture(u_opacityMap0, In.uv).r < OPACITY_THRESHOLD)
        discard;
        
    out_emission = u_emissionColor;
    
    if (u_hasEmissionMap > 0.0)
    {
        out_emission += texture(u_emissionMap0, In.uv).rgb;
        out_emission = clamp(out_emission, 0.0, 1.0);
    }

    out_diffuse = u_color.rgb;
    
    if (u_hasDiffuseTexture > 0.0)
    {
        out_diffuse = texture(u_diffuseTexture0, In.uv).rgb;
    }
    
    out_specular.rgb = u_specularColor;
    out_specular.a = packShininess(u_shininess);
    out_normal = normalize(In.normalW);
    
    if (u_hasNormalMap > 0.0)
    {
        vec3 normalSample = texture(u_normalMap0, In.uv).rgb;
        normalSample = 2.0 * normalSample - 1.0;
        
        vec3 tangent = normalize(In.tangentW);
        vec3 bitangent = normalize(In.bitangentW);
        out_normal = normalize(normalSample.x * tangent + normalSample.y * bitangent + normalSample.z * out_normal);
    }
    
    if (u_hasSpecularMap > 0.0)
    {
        out_specular.rgb = texture(u_specularMap0, In.uv).rgb;
    }
	
    out_normal = packNormal(out_normal);
}
