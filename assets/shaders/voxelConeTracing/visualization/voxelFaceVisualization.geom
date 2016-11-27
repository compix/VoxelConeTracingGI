#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform ivec3 u_imageMin;
uniform ivec3 u_regionMin;
// Expecting u_prevRegionMin and u_prevVolumeMax in the voxel coordinates of the current region
uniform ivec3 u_prevRegionMin;
uniform ivec3 u_prevRegionMax;

uniform int u_hasPrevClipmapLevel;
uniform int u_clipmapResolution;
uniform int u_clipmapLevel;
uniform float u_voxelSize;
uniform int u_hasMultipleFaces;
uniform int u_numColorComponents;

uniform mat4 u_viewProj;

uniform sampler3D u_3dTexture;

const float EPSILON = 0.00001;

#define VOXEL_TEXTURE_WITH_BORDER

const int BORDER_WIDTH = 1;

in Vertex
{
    int faceIdx;
} In[1];

out Geometry
{
    vec3 posW;
    vec4 color;
	vec2 uv;
} Out;

vec3 toWorld(ivec3 p)
{
	return p * u_voxelSize;
}

void createQuad(vec4 v0, vec4 v1, vec4 v2, vec4 v3)
{
	gl_Position = v0;
	Out.uv = vec2(0.0, 0.0);
    Out.posW = v0.xyz;
	EmitVertex();
	gl_Position = v1;
	Out.uv = vec2(0.0, 1.0);
    Out.posW = v1.xyz;
	EmitVertex();
	gl_Position = v3;
	Out.uv = vec2(1.0, 0.0);
    Out.posW = v3.xyz;
	EmitVertex();
	gl_Position = v2;
	Out.uv = vec2(1.0, 1.0);
    Out.posW = v2.xyz;
	EmitVertex();
	EndPrimitive();
}

void main()
{
	ivec3 pos = ivec3(gl_in[0].gl_Position.xyz);
	ivec3 posV = pos + u_regionMin;
	
	// Expecting u_prevRegionMin and u_prevVolumeMax in the voxel coordinates of the current region
	if (u_hasPrevClipmapLevel > 0 && (all(greaterThanEqual(posV, u_prevRegionMin)) && all(lessThan(posV, u_prevRegionMax))))
		return;

	// Same as: (u_imageMin + pos) % u_clipmapResolution - the bitwise version is faster than %
	ivec3 samplePos = (u_imageMin + pos) & (u_clipmapResolution - 1);
	
#ifdef VOXEL_TEXTURE_WITH_BORDER
	int resolution = u_clipmapResolution + BORDER_WIDTH * 2;
	samplePos += ivec3(BORDER_WIDTH);
#else
	int resolution = u_clipmapResolution;
#endif
	
	// Target correct clipmap level
	samplePos.y += u_clipmapLevel * resolution;
		
	vec4 color = texelFetch(u_3dTexture, samplePos + ivec3(resolution * In[0].faceIdx, 0, 0), 0);
	
	vec4 v0 = u_viewProj * vec4(toWorld(posV), 1.0);
	vec4 v1 = u_viewProj * vec4(toWorld(posV + ivec3(1, 0, 0)), 1.0);
	vec4 v2 = u_viewProj * vec4(toWorld(posV + ivec3(0, 1, 0)), 1.0);
	vec4 v3 = u_viewProj * vec4(toWorld(posV + ivec3(1, 1, 0)), 1.0);
	vec4 v4 = u_viewProj * vec4(toWorld(posV + ivec3(0, 0, 1)), 1.0);
	vec4 v5 = u_viewProj * vec4(toWorld(posV + ivec3(1, 0, 1)), 1.0);
	vec4 v6 = u_viewProj * vec4(toWorld(posV + ivec3(0, 1, 1)), 1.0);
	vec4 v7 = u_viewProj * vec4(toWorld(posV + ivec3(1, 1, 1)), 1.0);
    
    v0.y += u_voxelSize * 0.5;
    v1.y += u_voxelSize * 0.5;
    v2.y += u_voxelSize * 0.5;
    v3.y += u_voxelSize * 0.5;
    v4.y += u_voxelSize * 0.5;
    v5.y += u_voxelSize * 0.5;
    v6.y += u_voxelSize * 0.5;
    v7.y += u_voxelSize * 0.5;
    
    // To visualize borders of a clipmap level
    //if (any(equal(pos, ivec3(0))) || any(equal(pos, ivec3(u_clipmapResolution - 1))))
    //{
    //    for (int i = 0; i < 6; ++i)
    //        colors[i].rgb = vec3(0.0);
    //}
        
	
    Out.color = color;
    
	if (color.a >= EPSILON)
	{
        if (In[0].faceIdx == 0)
            createQuad(v0, v2, v6, v4);
        else if (In[0].faceIdx == 1)
            createQuad(v1, v5, v7, v3);
        else if (In[0].faceIdx == 2)
            createQuad(v0, v4, v5, v1);
        else if (In[0].faceIdx == 3)
            createQuad(v2, v3, v7, v6);
        else if (In[0].faceIdx == 4)
            createQuad(v0, v1, v3, v2);
        else if (In[0].faceIdx == 5)
            createQuad(v4, v6, v7, v5);
	}
}
