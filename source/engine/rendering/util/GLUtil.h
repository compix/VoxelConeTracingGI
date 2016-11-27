#pragma once
#include <GL/glew.h>

class Rect;

namespace GL
{
    void setViewport(const Rect& rect);

    bool isTextureBound(GLenum target, GLuint textureID);

    bool isShaderBound(GLuint programID);

    GLuint getCurrentShader();
}
