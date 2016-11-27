#version 100
precision mediump float;

attribute float in_isStart;

uniform vec3 u_startPos;
uniform vec3 u_endPos;

uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
    gl_Position = u_proj * u_view * vec4(in_isStart  < 0.5 ? u_startPos : u_endPos, 1.0);
}
