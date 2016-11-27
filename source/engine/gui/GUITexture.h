#pragma once
#include "GL/glew.h"
#include <string>

struct GUITexture
{
    GUITexture() {}

    GUITexture(const std::string& label, GLuint texID,
        GLint swizzleRed = GL_RED, GLint swizzleGreen = GL_GREEN, GLint swizzleBlue = GL_BLUE, GLint swizzleAlpha = GL_ALPHA)
        : label(label), texID(texID), swizzleMask{ swizzleRed, swizzleGreen, swizzleBlue, swizzleAlpha }
    {
        glBindTexture(GL_TEXTURE_2D, texID);
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, originalSwizzleMask);
    }

    void setSwizzleMask() const
    {
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }

    void resetSwizzleMask() const
    {
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, originalSwizzleMask);
    }

    GLint originalSwizzleMask[4];
    GLint swizzleMask[4]{ GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
    std::string label;
    GLuint texID{ 0 };
};
