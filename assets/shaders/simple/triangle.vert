#version 430
layout(location = 0) in float in_posIdx;

uniform mat4 u_modelViewProj;

uniform vec3 u_positions[3];

void main() 
{
	gl_Position = u_modelViewProj * vec4(u_positions[int(in_posIdx)], 1.0);
}
