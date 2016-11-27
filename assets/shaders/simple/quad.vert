#version 330
layout(location = 0) in vec3 in_pos;

uniform mat4 u_modelViewProj;

out vec2 v_texCoords;

void main() {
	gl_Position = u_modelViewProj * vec4(in_pos, 1.0);
	
	v_texCoords = in_pos.xy + vec2(0.5, 0.5);
}
