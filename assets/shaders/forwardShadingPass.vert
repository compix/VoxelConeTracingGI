#version 330
precision highp float;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec3 in_bitangent;
layout(location = 4) in vec2 in_uv;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_modelIT;
uniform mat4 u_lightView;
uniform mat4 u_lightProj;
uniform float u_lightZNear;
uniform float u_lightZFar;

out Vertex
{
	vec3 posW;
	vec3 normalW;
	vec3 tangentW;
	vec3 bitangentW;
	vec2 uv;
	vec3 lightViewPos;
	float lightZEye;
	float clipSpaceZ;
};

void main()
{
    posW = (u_model * vec4(in_pos, 1.0)).xyz;
    gl_Position = u_proj * u_view * u_model * vec4(in_pos, 1.0);
    clipSpaceZ = gl_Position.z;
    
    normalW = (u_modelIT * vec4(in_normal, 0.0)).xyz;
    tangentW = (u_modelIT * vec4(in_tangent, 0.0)).xyz;
    bitangentW = (u_modelIT * vec4(in_bitangent, 0.0)).xyz; // cross(tangentW, normalW);
    uv = in_uv;
    
    // Shadow mapping
	lightViewPos = (u_lightView * vec4(posW, 1.0)).xyz;
	lightZEye = lightViewPos.z;
	lightViewPos.z = lightZEye / (u_lightZFar - u_lightZNear);
	lightViewPos.xy = (u_lightProj * vec4(lightViewPos.xy, 0.0, 1.0)).xy;
	
	// Map from [-1.0, 1.0] -> [0.0, 1.0]
	lightViewPos.xy = lightViewPos.xy * 0.5 + 0.5;
}
