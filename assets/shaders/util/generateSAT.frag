#version 430
precision highp float;

in Vertex
{
	vec2 texCoords;
} In;

const int NUM_TEXTURE_FETCHES = 16;

//uniform sampler2D u_texture;
uniform usampler2D u_texture;

uniform vec2 u_textureSize;
uniform int u_passIdx;
uniform int u_isHorizontalPhase;

layout (location = 0) out uvec4 out_color;

// Algorithm according to "Fast Summed-Area Table Generation and its Applications" by Hensley et al.
void main() 
{
	vec2 pixelSize = vec2(1.0 / u_textureSize.x, 1.0 / u_textureSize.y);
	
	//ivec2 uv = ivec2(In.texCoords * u_textureSize);
	vec2 uv = In.texCoords;
	//vec4 sum = texture(u_texture, uv);
	uvec4 sum = texture(u_texture, uv);
	//vec4 sum = texelFetch(u_texture, uv, 0);
	
	if (u_isHorizontalPhase > 0)
	{
		for (int i = 0; i < NUM_TEXTURE_FETCHES - 1; ++i)
		{
			uv.x = In.texCoords.x - (i + 1) * pow(NUM_TEXTURE_FETCHES, u_passIdx) * pixelSize.x;
			//uv.x = int(In.texCoords.x * u_textureSize) - int((i + 1) * pow(NUM_TEXTURE_FETCHES, u_passIdx));
			
			sum += texture(u_texture, uv);
			//sum += texelFetch(u_texture, uv, 0);
		}
	}
	else
	{
		for (int i = 0; i < NUM_TEXTURE_FETCHES - 1; ++i)
		{
			uv.y = In.texCoords.y - (i + 1) * pow(NUM_TEXTURE_FETCHES, u_passIdx) * pixelSize.y;
			
			sum += texture(u_texture, uv);
		}
	}
	
	out_color = sum;
}
