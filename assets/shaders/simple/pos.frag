#version 100
precision highp float;

uniform vec3 u_color;

void main() 
{
	gl_FragColor = vec4(u_color, 1.0);
}
