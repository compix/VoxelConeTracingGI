#pragma once
#include <engine/rendering/Framebuffer.h>
#include <engine/rendering/geometry/Mesh.h>
#include <engine/rendering/shader/Shader.h>
#include <engine/util/colors.h>
#include <engine/rendering/renderer/SimpleMeshRenderer.h>

class SATGenerator
{
public:
    SATGenerator(int texWidth, int texHeight);

    GLuint generateSAT(GLuint texture);

private:
    void swapBuffers();

    uint32_t getBackBufferIdx() const { return (m_curBufferIdx + 1) % 2; }
    Framebuffer* getBackBuffer() const { return m_framebuffers[getBackBufferIdx()].get(); }
    void createFramebuffers(int width, int height);

private:
    std::unique_ptr<Framebuffer> m_framebuffers[2];
    int m_width;
    int m_height;
    uint32_t m_curBufferIdx{0};
    std::shared_ptr<SimpleMeshRenderer> m_fullscreenQuadRenderer;

    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Shader> m_quadShaderToInteger;
    std::shared_ptr<Shader> m_quadShader;
};
