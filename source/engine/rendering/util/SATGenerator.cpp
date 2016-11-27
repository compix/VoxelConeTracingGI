#include "SATGenerator.h"
#include <engine/rendering/util/GLUtil.h>
#include <engine/geometry/Rect.h>
#include <engine/resource/ResourceManager.h>
#include <engine/rendering/renderer/MeshRenderers.h>

SATGenerator::SATGenerator(int texWidth, int texHeight)
    : m_width(texWidth), m_height(texHeight)
{
    m_fullscreenQuadRenderer = MeshRenderers::fullscreenQuad();
    m_shader = ResourceManager::getShader("shaders/util/generateSAT.vert", "shaders/util/generateSAT.frag");
    m_quadShader = ResourceManager::getShader("shaders/simple/fullscreenQuad.vert", "shaders/simple/fullscreenQuad.frag", {"in_pos"});
    m_quadShaderToInteger = ResourceManager::getShader("shaders/util/fullscreenQuadToInteger.vert", "shaders/util/fullscreenQuadToInteger.frag");
    createFramebuffers(m_width, m_height);
}

GLuint SATGenerator::generateSAT(GLuint texture)
{
    // Algorithm according to "Fast Summed-Area Table Generation and its Applications" by Hensley et al.
    float w = float(m_width);
    float h = float(m_height);

    int numTextureFetches = 16;
    float logN = log(float(numTextureFetches));

    int n = static_cast<int>(ceil(log(w) / logN));
    int m = static_cast<int>(ceil(log(h) / logN));

    glDisable(GL_DEPTH_TEST);

    // Render texture to the first buffer
    m_framebuffers[0]->bind();
    GL::setViewport(Rect(0.f, 0.f, w, h));

    m_quadShaderToInteger->bind();
    m_quadShaderToInteger->setFloat("u_isGrayscale", 0.f);
    m_quadShaderToInteger->bindTexture2D(texture, "u_textureDiffuse");
    m_fullscreenQuadRenderer->bindAndRender();

    m_framebuffers[0]->unbind();

    m_shader->bind();
    m_shader->setVector("u_textureSize", glm::vec2(w, h));
    m_shader->setInt("u_isHorizontalPhase", 1);

    m_curBufferIdx = 0;

    // Horizontal phase
    for (int i = 0; i < n; ++i)
    {
        auto fb = getBackBuffer();
        fb->bind();
        GL::setViewport(Rect(0.f, 0.f, w, h));
        m_shader->bindTexture2D(m_framebuffers[m_curBufferIdx]->getRenderTexture(GL_COLOR_ATTACHMENT0), "u_texture");
        m_shader->setInt("u_passIdx", i);
        m_fullscreenQuadRenderer->bindAndRender();

        swapBuffers();
        fb->unbind();
    }

    m_shader->setInt("u_isHorizontalPhase", 0);

    // Vertical phase
    for (int i = 0; i < m; ++i)
    {
        auto fb = getBackBuffer();
        fb->bind();
        GL::setViewport(Rect(0.f, 0.f, w, h));
        m_shader->bindTexture2D(m_framebuffers[m_curBufferIdx]->getRenderTexture(GL_COLOR_ATTACHMENT0), "u_texture");
        m_shader->setInt("u_passIdx", i);
        m_fullscreenQuadRenderer->bindAndRender();

        swapBuffers();
        fb->unbind();
    }

    return m_framebuffers[m_curBufferIdx]->getRenderTexture(GL_COLOR_ATTACHMENT0);
}

void SATGenerator::swapBuffers()
{
    m_curBufferIdx = (m_curBufferIdx + 1) % 2;
}

void SATGenerator::createFramebuffers(int width, int height)
{
    for (int i = 0; i < 2; ++i)
    {
        m_framebuffers[i] = std::make_unique<Framebuffer>();
        m_framebuffers[i]->bind();

        std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>();
        //tex->create(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, Texture2DSettings::Custom);
        tex->create(width, height, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, Texture2DSettings::Custom);

        tex->bind();
        tex->setParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        tex->setParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        tex->setParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        tex->setParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        //float zero[4]{ 0.f, 0.f, 0.f, 0.f };
        //glTextureParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zero);
        GL_ERROR_CHECK();

        m_framebuffers[i]->attachRenderTexture2D(tex, GL_COLOR_ATTACHMENT0);
        m_framebuffers[i]->setDrawBuffers();
        m_framebuffers[i]->checkFramebufferStatus();
        m_framebuffers[i]->unbind();
    }
}
