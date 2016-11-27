#pragma once
#include <GL/glew.h>
#include <string>

using TextureID = GLuint;

enum class Texture3DSettings
{
    Default,
    Custom
};

class Texture3D
{
public:
    Texture3D() { }

    ~Texture3D();

    operator TextureID() const noexcept { return m_glId; }

    void create(GLsizei width = 128, GLsizei height = 128, GLsizei depth = 128, GLint internalFormat = GL_RGBA8, GLenum format = GL_RGBA,
                GLenum pixelType = GL_UNSIGNED_BYTE, Texture3DSettings settings = Texture3DSettings::Default);

    GLsizei getWidth() const noexcept { return m_width; }

    GLsizei getHeight() const noexcept { return m_height; }

    int getChannelCount() const { return m_channels; }

    GLenum getTextureFormat() const { return m_format; }

    bool isValid() const { return m_glId != 0; }

    void setParameteri(GLenum name, GLint value) const;

    void resize(GLsizei width, GLsizei height);

    void bind();
private:
    TextureID m_glId{0};
    GLsizei m_width{0};
    GLsizei m_height{0};
    GLsizei m_depth{0};
    int m_channels{0}; // e.g. GL_LUMINANCE = 1 channel, GL_LUMINANCE_ALPHA = 2 channels, GL_RGB = 3 channels, GL_RGBA = 4 channels
    GLenum m_format{0};
    GLint m_internalFormat{0};
    GLenum m_pixelType{0};
};
