#version 430

in Vertex
{
    vec3 normalW;
    vec3 tangentW;
    vec3 bitangentW;
    vec2 uv;
} In;

uniform sampler2D u_opacityMap0;
uniform float u_hasOpacityMap;
uniform uint u_entityID;
uniform uint u_entityVersion;

layout (location = 0) out uvec2 out_entityID;

void main() 
{
    if (u_hasOpacityMap > 0.0 && texture(u_opacityMap0, In.uv).r < 0.1)
        discard;

	out_entityID = uvec2(u_entityID, u_entityVersion);
}
