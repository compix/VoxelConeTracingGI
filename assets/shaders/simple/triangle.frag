#version 430

uniform vec4 u_color;
uniform vec3 u_normalW;

layout(location = 0) out vec4 out_color;

vec3 computeDirectionalLight(vec3 direction, vec3 lightDiffuse, vec3 normal)
{
	return max(dot(normalize(-direction), normal), 0.0) * lightDiffuse;
}

void main() 
{
    //vec3 keyLightDir  = vec3(0.57735, -0.57735, 0.57735);
    //vec3 fillLighDir = vec3(-0.57735, -0.57735, 0.57735);
    //vec3 backLightDir = vec3(0.0, -0.707, -0.707);
    //
    //vec3 lightDiffuse = vec3(0.5, 0.5, 0.5);
    //
    //vec3 color = computeDirectionalLight(keyLightDir, lightDiffuse, u_normalW);
    //color += computeDirectionalLight(fillLighDir, lightDiffuse, u_normalW);
    //color += computeDirectionalLight(backLightDir, lightDiffuse, u_normalW);
    
	//out_color = u_color * vec4(color, 1.0);
    
    out_color = u_color;
}
