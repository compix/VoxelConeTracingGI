#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

uniform int u_clipmapLevel;
uniform int u_faceCount;
uniform ivec3 u_resolution;
uniform float u_texelSize = 0.125;
uniform float u_padding = 0.0;
uniform vec3 u_position = vec3(0.0);

uniform mat4 u_viewProj;

uniform sampler3D u_3dTexture;

const float EPSILON = 0.00001;

out Geometry
{
    vec4 color;
	vec2 uv;
} Out;

vec3 toWorld(vec3 p, vec3 offset)
{
	return (p + offset) * u_texelSize + u_position + p * u_padding;
}

void createQuad(vec4 v0, vec4 v1, vec4 v2, vec4 v3)
{
	gl_Position = v0;
	Out.uv = vec2(0.0, 0.0);
	EmitVertex();
	gl_Position = v1;
	Out.uv = vec2(0.0, 1.0);
	EmitVertex();
	gl_Position = v3;
	Out.uv = vec2(1.0, 0.0);
	EmitVertex();
	gl_Position = v2;
	Out.uv = vec2(1.0, 1.0);
	EmitVertex();
	EndPrimitive();
}

void main()
{
	vec3 pos = gl_in[0].gl_Position.xyz;
	ivec3 samplePos = ivec3(pos);
	
	// Target correct clipmap level
	samplePos.y += u_clipmapLevel * u_resolution.y;
		
	vec4 colors[6] = vec4[](vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));
	
	for (int i = 0; i < u_faceCount; ++i)
	{
		colors[i] = texelFetch(u_3dTexture, samplePos + ivec3(u_resolution.x * i, 0, 0), 0);
	}
	
	vec4 v0 = u_viewProj * vec4(toWorld(pos, vec3(0.0)), 1.0);
	vec4 v1 = u_viewProj * vec4(toWorld(pos, vec3(1, 0, 0)), 1.0);
	vec4 v2 = u_viewProj * vec4(toWorld(pos, vec3(0, 1, 0)), 1.0);
	vec4 v3 = u_viewProj * vec4(toWorld(pos, vec3(1, 1, 0)), 1.0);
	vec4 v4 = u_viewProj * vec4(toWorld(pos, vec3(0, 0, 1)), 1.0);
	vec4 v5 = u_viewProj * vec4(toWorld(pos, vec3(1, 0, 1)), 1.0);
	vec4 v6 = u_viewProj * vec4(toWorld(pos, vec3(0, 1, 1)), 1.0);
	vec4 v7 = u_viewProj * vec4(toWorld(pos, vec3(1, 1, 1)), 1.0);
	
	// X Axis left face of the cube
	if (colors[0].a >= EPSILON)
	{
		Out.color = colors[0];
		createQuad(v0, v2, v6, v4);
	}
	
	// X Axis right face of the cube
	if (colors[1].a >= EPSILON)
	{
		Out.color = colors[1];
		createQuad(v1, v5, v7, v3);
	}
	
	// Y Axis bottom face of the cube
	if (colors[2].a >= EPSILON)
	{
		Out.color = colors[2];
		createQuad(v0, v4, v5, v1);
	}
	
	// Y Axis top face of the cube
	if (colors[3].a >= EPSILON)
	{
		Out.color = colors[3];
		createQuad(v2, v3, v7, v6);
	}
	
	// Z Axis front face of the cube
	if (colors[4].a >= EPSILON)
	{
		Out.color = colors[4];
		createQuad(v0, v1, v3, v2);
	}
	
	// Z Axis back face of the cube
	if (colors[5].a >= EPSILON)
	{
		Out.color = colors[5];
		createQuad(v4, v6, v7, v5);
	}
}
