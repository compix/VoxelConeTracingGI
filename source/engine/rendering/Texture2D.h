#pragma once
#include <GL/glew.h>
#include <string>

using TextureID = GLuint;

enum class Texture2DSettings
{
    S_T_REPEAT_MIN_MAG_LINEAR,
    S_T_REPEAT_MIN_MIPMAP_LINEAR_MAG_LINEAR,
    S_T_CLAMP_TO_BORDER_MIN_MIPMAP_LINEAR_MAG_LINEAR,
    S_T_REPEAT_MIN_MAG_NEAREST,
    S_T_CLAMP_TO_BORDER_MIN_MAX_NEAREST,
    S_T_CLAMP_TO_BORDER_MIN_MAX_LINEAR,
    S_T_REPEAT_ANISOTROPIC,
    Custom
};

class Texture2D
{
public:
    Texture2D() { }

    explicit Texture2D(const std::string& path, Texture2DSettings settings = Texture2DSettings::S_T_REPEAT_MIN_MAG_LINEAR);

    ~Texture2D();

    operator TextureID() const noexcept { return m_glId; }

    void create(GLsizei width = 1024, GLsizei height = 1024, GLint internalFormat = GL_RGBA8, GLenum format = GL_RGBA,
                GLenum pixelType = GL_UNSIGNED_BYTE, Texture2DSettings settings = Texture2DSettings::S_T_REPEAT_MIN_MAG_LINEAR, const void* pixels = nullptr);

    void createMultisampled(uint8_t numSamples, GLsizei width, GLsizei height, GLint internalFormat);

    void load(const std::string& path, Texture2DSettings settings = Texture2DSettings::S_T_REPEAT_MIN_MAG_LINEAR);

    GLsizei getWidth() const noexcept { return m_width; }

    GLsizei getHeight() const noexcept { return m_height; }

    int getChannelCount() const { return m_channels; }

    GLenum getTextureFormat() const { return m_format; }

    bool isValid() const { return m_glId != 0; }

    void setParameteri(GLenum name, GLint value) const;

    void resize(GLsizei width, GLsizei height);

    void bind() const { glBindTexture(GL_TEXTURE_2D, m_glId); }
private:
    void applySettings(Texture2DSettings settings);

private:
    uint8_t m_numSamples{1};
    TextureID m_glId{0};
    GLsizei m_width{0};
    GLsizei m_height{0};
    int m_channels{0}; // e.g. GL_RGB = 3 channels, GL_RGBA = 4 channels
    GLenum m_format{0};
    GLint m_internalFormat{0};
    GLenum m_pixelType{0};
    GLenum m_target{0};
};
