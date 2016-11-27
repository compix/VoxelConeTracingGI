#version 100
precision mediump float;

attribute vec3 in_pos;

varying vec2 v_texCoords;

void main() {
	gl_Position = vec4(in_pos, 1.0);
	v_texCoords = in_pos.xy * 0.5 + vec2(0.5, 0.5);
}
