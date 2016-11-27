#version 100
precision highp float;

attribute vec3 in_pos;
attribute vec3 in_normal;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_modelIT;

varying vec3 v_normalW;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(in_pos, 1.0);
    
    v_normalW = (u_modelIT * vec4(in_normal, 0.0)).xyz;
}
