#pragma once
#include <GL/glew.h>
#include <string>
#include <memory>
#include <vector>
#include "Texture2D.h"
#include <unordered_map>
#include <cstddef>

class Framebuffer
{
public:
    Framebuffer();
    Framebuffer(GLsizei width, GLsizei height, bool hasRenderTexture = false, GLenum pixelType = GL_UNSIGNED_BYTE, bool hasDepthBuffer = false);
    ~Framebuffer();

    operator GLuint() const noexcept { return m_fbo; }

    /**
    * Starts drawing into this framebuffer.
    * All subsequent draw calls will affect this framebuffer.
    * Make sure to call end() when done drawing to this framebuffer.
    */
    void begin() const;

    void begin(GLsizei width, GLsizei height) const;

    void bind() const;
    void unbind() const;

    /**
    * Ends drawing into this framebuffer.
    * All subsequent draw calls will affect the default framebuffer 0.
    */
    void end() const;

    GLuint getRenderTexture(GLenum colorAttachment = GL_COLOR_ATTACHMENT0) noexcept { return *m_renderTextures[colorAttachment]; }

    GLuint getDepthTexture(uint32_t idx = 0) const noexcept { return *m_depthTextures[idx]; }

    std::size_t getRenderTextureCount() const noexcept { return m_renderTextures.size(); }

    std::size_t getDepthBufferCount() const { return m_depthTextures.size(); }

    void resize(GLsizei width, GLsizei height);

    /**
    * Saves the render texture of this buffer to the given file in binary format.
    * If the filename does not exist then a new file will be created.
    */
    void saveRenderTexture(const std::string& filename);

    /**
    * Loads the render texture from the specified filename in binary format.
    */
    void loadRenderTexture(const std::string& filename);

    void bindDepthBuffer(uint32_t idx = 0);

    void attachDepthBuffer(GLsizei width, GLsizei height);

    void attachDepthBuffer(std::shared_ptr<Texture2D> texture);
    void attachDepthBufferMultisample(std::shared_ptr<Texture2D> texture);
    void addDepthBuffer(std::shared_ptr<Texture2D> texture);

    void attachRenderTexture2D(std::shared_ptr<Texture2D> texture, GLenum attachment = GL_COLOR_ATTACHMENT0);
    void attachRenderTexture2DMultisample(std::shared_ptr<Texture2D> texture, GLenum attachment = GL_COLOR_ATTACHMENT0);

    bool hasDepthBuffer() const { return m_depthTextures.size() > 0; }

    bool hasRenderTextures() const { return m_renderTextures.size() > 0; }

    void checkFramebufferStatus();

    GLsizei getWidth() const { return m_width; }

    GLsizei getHeight() const { return m_height; }

    void setDrawBuffers();
private:
    std::unordered_map<GLenum, std::shared_ptr<Texture2D>> m_renderTextures;
    std::vector<std::shared_ptr<Texture2D>> m_depthTextures;

    GLsizei m_width{0};
    GLsizei m_height{0};

    GLuint m_fbo{0};
};
