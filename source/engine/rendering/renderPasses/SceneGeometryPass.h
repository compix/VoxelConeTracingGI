#pragma once
#include <engine/rendering/shader/Shader.h>
#include <engine/rendering/Framebuffer.h>
#include <engine/rendering/architecture/RenderPass.h>
#include "engine/input/Input.h"
#include <cstddef>

class MeshRenderer;

/**
 * Renders the scene into G-Buffers for deferred shading.
 */
class SceneGeometryPass : public RenderPass, public InputHandler
{
public:
    SceneGeometryPass();
    void render() const;

    GLuint getRenderTexture(GLenum colorAttachment = GL_COLOR_ATTACHMENT0) const noexcept { return m_framebuffer->getRenderTexture(colorAttachment); }
    GLuint getDepthTexture() const noexcept { return m_framebuffer->getDepthTexture(); }
    std::size_t getRenderTextureCount() const noexcept { return m_framebuffer->getRenderTextureCount(); }

    void update() override;

protected:
    void onWindowEvent(const SDL_WindowEvent& windowEvent) override;

private:
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::shared_ptr<Shader> m_shader;
};
