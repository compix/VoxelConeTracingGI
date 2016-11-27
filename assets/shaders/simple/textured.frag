#version 430

in Vertex
{
	vec3 posW;
	vec3 normalW;
	vec3 tangentW;
	//vec3 bitangentW;
	vec2 uv;
} In;

uniform vec4 u_color;

uniform vec4 u_textureRegion;

uniform sampler2D u_diffuseTexture0;
uniform float u_lod;

layout(location = 0) out vec4 out_color;

void main() 
{
	//out_color = texture(u_diffuseTexture0, vec2(In.uv.x, 1.0 - In.uv.y)) * u_color;
    
    vec2 extent = u_textureRegion.zw - u_textureRegion.xy;
    vec2 uv = In.uv * extent;
    uv = uv + u_textureRegion.xy;
    
    out_color = textureLod(u_diffuseTexture0, vec2(uv.x, 1.0 - uv.y), u_lod) * u_color;
}
