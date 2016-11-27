#version 100
precision highp float;

// Attributes
varying vec3 v_normalW;

uniform vec4 u_color;

vec3 computeDirectionalLight(vec3 direction, vec3 lightDiffuse, vec3 normal)
{
	return max(dot(normalize(-direction), normal), 0.0) * lightDiffuse;
}

void main() 
{
    vec3 keyLightDir  = vec3(0.57735, -0.57735, 0.57735);
    vec3 fillLighDir = vec3(-0.57735, -0.57735, 0.57735);
    vec3 backLightDir = vec3(0.0, -0.707, -0.707);
    
    vec3 normal = normalize(v_normalW);
    
    vec3 lightDiffuse = vec3(0.5, 0.5, 0.5);
    
    vec3 color = computeDirectionalLight(keyLightDir, lightDiffuse, normal);
    color += computeDirectionalLight(fillLighDir, lightDiffuse, normal);
    color += computeDirectionalLight(backLightDir, lightDiffuse, normal);
    
	gl_FragColor = u_color * vec4(color, 1.0);
}
