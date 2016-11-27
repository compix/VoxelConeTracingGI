#ifndef COMMON_GLSL
#define COMMON_GLSL

ivec3 computeVoxelFaceIndices(vec3 direction)
{
	return ivec3(direction.x > 0.0 ? 0 : 1,
				 direction.y > 0.0 ? 2 : 3,
			     direction.z > 0.0 ? 4 : 5);
	
	// Branchless version - not necessarily faster
	//return ivec3(1, 3, 5) - (ivec3((sign(direction) + 1)) / 2);
}

int getDominantAxisIdx(vec3 v0, vec3 v1, vec3 v2)
{
    vec3 aN = abs(cross(v1 - v0, v2 - v0));
    
    if (aN.x > aN.y && aN.x > aN.z)
        return 0;
        
    if (aN.y > aN.z)
        return 1;

    return 2;
}

const vec3[] MAIN_AXES = {
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
};

const float EPSILON = 0.000001;
const float PI = 3.14159265;

#endif