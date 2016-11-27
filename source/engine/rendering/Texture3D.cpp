#include "Texture3D.h"
#include <engine/util/Logger.h>
#include "util/GLUtil.h"

Texture3D::~Texture3D()
{
    if (isValid())
    {
        glDeleteTextures(1, &m_glId);
        m_glId = 0;
    }
}

void Texture3D::create(GLsizei width, GLsizei height, GLsizei depth, GLint internalFormat, GLenum format, GLenum pixelType, Texture3DSettings settings)
{
    if (isValid())
    {
        glDeleteTextures(1, &m_glId);
        m_glId = 0;
    }

    m_format = format;
    m_internalFormat = internalFormat;
    m_pixelType = pixelType;
    m_width = width;
    m_height = height;
    m_depth = depth;

    switch (format)
    {
    case GL_LUMINANCE:
    case GL_DEPTH_COMPONENT:
        m_channels = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        m_channels = 2;
        break;
    case GL_RGB:
        m_channels = 3;
        break;
    case GL_RGBA:
        m_channels = 4;
        break;
    default: break;
    }

    glGenTextures(1, &m_glId);

    glBindTexture(GL_TEXTURE_3D, m_glId);
    glTexImage3D(GL_TEXTURE_3D, 0, m_internalFormat, m_width, m_height, m_depth, 0, m_format, m_pixelType, nullptr);

    switch (settings)
    {
    case Texture3DSettings::Default:
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case Texture3DSettings::Custom: break;
    default: break;
    }
}

void Texture3D::setParameteri(GLenum name, GLint value) const
{
    assert(isValid());
    assert(GL::isTextureBound(GL_TEXTURE_3D, m_glId));

    glTexParameteri(GL_TEXTURE_3D, name, value);
}

void Texture3D::resize(GLsizei width, GLsizei height)
{
    m_width = width;
    m_height = height;

    glBindTexture(GL_TEXTURE_3D, m_glId);
    glTexImage3D(GL_TEXTURE_3D, 0, m_internalFormat, m_width, m_height, m_depth, 0, m_format, m_pixelType, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture3D::bind()
{
    glBindTexture(GL_TEXTURE_3D, m_glId);
}
