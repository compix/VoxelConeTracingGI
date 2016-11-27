#include "convert.h"

size_t convert::sizeFromGLType(GLenum type)
{
    switch (type)
    {
    case GL_FLOAT:
        return sizeof(GLfloat);
    case GL_BYTE:
        return sizeof(GLbyte);
    case GL_SHORT:
        return sizeof(GLshort);
    case GL_INT:
        return sizeof(GLint);
    case GL_UNSIGNED_BYTE:
        return sizeof(GLubyte);
    case GL_UNSIGNED_SHORT:
        return sizeof(GLushort);
    case GL_UNSIGNED_INT:
        return sizeof(GLuint);
    default:
        assert(false);
    }

    return 0;
}
