#version 430
precision highp float;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_instancePos;
layout(location = 2) in vec4 in_instanceColor;

uniform float u_scale;
uniform mat4 u_proj;
uniform mat4 u_view;

out Vertex
{
    vec4 color;
};

const mat3 ROTATIONS[] = {
	mat3(vec3(0.0, 0.0, 1.0),
		 vec3(0.0, 1.0, 0.0),
		 vec3(-1.0, 0.0, 0.0)),
	mat3(vec3(1.0, 0.0, 0.0),
		 vec3(0.0, 0.0, -1.0),
		 vec3(0.0, 1.0, 0.0)),
	mat3(vec3(1.0, 0.0, 0.0),
		 vec3(0.0, 1.0, 0.0),
		 vec3(0.0, 0.0, 1.0))		 
};

void main() 
{
	vec3 pos = in_pos * u_scale;
	int faceIdx = int(in_instancePos.w);
	pos = ROTATIONS[faceIdx] * pos;
	
	gl_Position = u_proj * u_view * vec4(pos + in_instancePos.xyz, 1.0);
	color = in_instanceColor;
	//texCoords = in_pos.xy * 0.5 + vec2(0.5, 0.5);
}
