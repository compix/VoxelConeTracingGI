#pragma once
#include <GL/glew.h>
#include <engine/rendering/shader/Shader.h>
#include <memory>

class Mipmapper
{
public:
    static void init();
    static void generateMipmap3D(GLuint texture, int startDimension);

private:
    static std::shared_ptr<Shader> m_mipmapShader;
};
