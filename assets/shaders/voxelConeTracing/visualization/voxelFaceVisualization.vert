#version 430

layout(location = 0) in vec4 in_pos;

out Vertex
{
    int faceIdx;
};

void main()
{
	gl_Position = vec4(in_pos.xyz, 1.0);
    faceIdx = int(in_pos.w);
}
