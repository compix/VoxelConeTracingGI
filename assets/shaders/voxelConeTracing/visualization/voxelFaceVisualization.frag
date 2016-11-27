#version 430

in Geometry
{
    vec3 posW;
	vec4 color;
	vec2 uv;
} In;

layout (location = 0) out vec4 out_color;

const float EPSILON = 0.00001;

uniform vec3 u_eyePosW;
uniform float u_borderWidth = 0.05;
uniform vec4 u_borderColor = vec4(0.5, 0.5, 0.5, 1.0);
uniform float u_alpha;

void main() 
{
	out_color = In.color;
    vec3 n = normalize(u_eyePosW - In.posW);
    float a = dot(n, vec3(0,1,0));
    
	float d = distance(u_eyePosW, In.posW);
    float borderWidth = u_borderWidth;
    
	if (borderWidth > EPSILON)
		out_color = mix(u_borderColor, In.color, min(min(In.uv.x, min(In.uv.y, min((1.0 - In.uv.x), (1.0 - In.uv.y)))) / borderWidth, 1.0));
		
	out_color.a = u_alpha;
}
