#version 330
precision mediump float;

in vec2 v_texCoords;

uniform sampler2D u_textureDiffuse;

layout(location = 0) out vec4 out_color;

void main() 
{
	out_color = texture2D(u_textureDiffuse, v_texCoords);
}
