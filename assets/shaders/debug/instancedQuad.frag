#version 430
precision highp float;

in Vertex
{
    vec4 color;
} In;

layout (location = 0) out vec4 out_color;

void main() 
{
	out_color = In.color;
}
