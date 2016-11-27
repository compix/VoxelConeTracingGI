#version 430
precision highp float;

in Vertex
{
	vec2 texCoords;
} In;

uniform sampler2D u_textureDiffuse;

layout(location = 0) out uvec4 out_color;

void main() 
{
    vec4 color = texture(u_textureDiffuse, In.texCoords);
		
	out_color = uvec4(color * pow(2, 14));
}
