#version 430
#extension GL_ARB_shading_language_include : enable

#define CONSERVATIVE_VOXELIZATION

#include "/voxelConeTracing/voxelizationGeom.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in Vertex
{
    vec3 normalW;
    vec2 uv;
} In[3];

out Geometry
{
    vec3 normalW;
    vec2 uv;
} Out;

void main()
{
	vec4 positionsClip[3];
	cvGeometryPass(positionsClip);
	
	for (int i = 0; i < 3; ++i)
    {
        Out.uv = In[i].uv;
        Out.normalW = In[i].normalW;
        
		cvEmitVertex(positionsClip[i]);
    }

    EndPrimitive();
}
