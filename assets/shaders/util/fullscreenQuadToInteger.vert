#version 430
precision highp float;

layout(location = 0) in vec3 in_pos;

out Vertex
{
	vec2 texCoords;
};

void main() {
	gl_Position = vec4(in_pos, 1.0);
	texCoords = in_pos.xy * 0.5 + vec2(0.5, 0.5);
}
