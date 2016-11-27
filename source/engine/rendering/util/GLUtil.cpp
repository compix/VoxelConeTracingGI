#include "GLUtil.h"
#include <GL/glew.h>
#include <engine/geometry/Rect.h>

void GL::setViewport(const Rect& rect)
{
    glScissor(GLint(rect.minX()), GLint(rect.minY()), GLsizei(rect.width()), GLsizei(rect.height()));
    glViewport(GLint(rect.minX()), GLint(rect.minY()), GLsizei(rect.width()), GLsizei(rect.height()));
}

bool GL::isTextureBound(GLenum target, GLuint textureID)
{
    GLint id = 0;

    switch (target)
    {
    case GL_TEXTURE_2D:
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &id);
        break;
    case GL_TEXTURE_3D:
        glGetIntegerv(GL_TEXTURE_BINDING_3D, &id);
        break;
    default:
        assert(false);
    }

    return GLuint(id) == textureID;
}

bool GL::isShaderBound(GLuint programID)
{
    GLint id;
    glGetIntegerv(GL_CURRENT_PROGRAM, &id);

    return id == GLint(programID);
}

GLuint GL::getCurrentShader()
{
    GLint id;
    glGetIntegerv(GL_CURRENT_PROGRAM, &id);

    return GLuint(id);
}
