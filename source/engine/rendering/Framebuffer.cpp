#include "Framebuffer.h"
#include <fstream>
#include <engine/util/Logger.h>
#include <engine/util/file.h>
#include <engine/util/convert.h>

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &m_fbo);
}

Framebuffer::Framebuffer(GLsizei width, GLsizei height, bool hasRenderTexture, GLenum pixelType, bool hasDepthBuffer)
    : m_width(width), m_height(height)
{
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (hasRenderTexture)
    {
        GLint internalFormat = 0;

        switch (pixelType)
        {
        case GL_FLOAT:
            internalFormat = GL_RGBA32F;
            break;
        case GL_UNSIGNED_BYTE:
            internalFormat = GL_RGBA8;
            break;
        default:
            LOG_ERROR("Framebuffer: Unsupported pixel type: " << pixelType);
        }

        auto renderTexture = std::make_shared<Texture2D>();
        renderTexture->create(width, height, internalFormat, GL_RGBA, pixelType);

        attachRenderTexture2D(renderTexture, GL_COLOR_ATTACHMENT0);

        setDrawBuffers();

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (hasDepthBuffer)
        attachDepthBuffer(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_ERROR_CHECK();
}


Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &m_fbo);
}

void Framebuffer::begin() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glScissor(0, 0, m_width, m_height);
}

void Framebuffer::begin(GLsizei width, GLsizei height) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::end() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(GLsizei width, GLsizei height)
{
    for (auto& p : m_renderTextures) { p.second->resize(width, height); }

    for (auto& d : m_depthTextures) { d->resize(width, height); }

    m_width = width;
    m_height = height;
}

void Framebuffer::saveRenderTexture(const std::string& filename) 
{ 
    // TODO: Implement
}

void Framebuffer::loadRenderTexture(const std::string& filename) 
{ 
    // TODO: Implement
}

void Framebuffer::bindDepthBuffer(uint32_t idx)
{
    assert(idx < m_depthTextures.size());

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *m_depthTextures[idx], 0);
}

void Framebuffer::attachDepthBuffer(GLsizei width, GLsizei height)
{
    auto depthTexture = std::make_shared<Texture2D>();

    depthTexture->create(width, height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthTexture, 0);
    m_depthTextures.push_back(depthTexture);

    GL_ERROR_CHECK();
}

void Framebuffer::attachDepthBuffer(std::shared_ptr<Texture2D> texture)
{
    m_depthTextures.push_back(texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *texture, 0);

    GL_ERROR_CHECK();
}

void Framebuffer::attachDepthBufferMultisample(std::shared_ptr<Texture2D> texture)
{
    m_depthTextures.push_back(texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, *texture, 0);

    GL_ERROR_CHECK();
}

void Framebuffer::addDepthBuffer(std::shared_ptr<Texture2D> texture) { m_depthTextures.push_back(texture); }

void Framebuffer::attachRenderTexture2D(std::shared_ptr<Texture2D> texture, GLenum attachment)
{
    m_renderTextures[attachment] = texture;
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, *texture, 0);

    GL_ERROR_CHECK();
}

void Framebuffer::attachRenderTexture2DMultisample(std::shared_ptr<Texture2D> texture, GLenum attachment)
{
    m_renderTextures[attachment] = texture;
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D_MULTISAMPLE, *texture, 0);

    GL_ERROR_CHECK();
}

void Framebuffer::checkFramebufferStatus()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERROR("Framebuffer incomplete - Error Code: " << status);
    }
}

void Framebuffer::setDrawBuffers()
{
    if (!hasRenderTextures())
        return;

    // Specify buffers into which outputs from frag shader will be written
    GLenum* drawBuffers = new GLenum[m_renderTextures.size()];

    for (auto& p : m_renderTextures)
    {
        drawBuffers[p.first - GL_COLOR_ATTACHMENT0] = p.first;
    }

    glDrawBuffers(GLsizei(m_renderTextures.size()), drawBuffers);

    delete[] drawBuffers;
}
