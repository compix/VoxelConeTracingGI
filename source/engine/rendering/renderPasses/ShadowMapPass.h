#pragma once
#include <memory>
#include <engine/rendering/Framebuffer.h>
#include <glm/mat4x2.hpp>
#include <engine/rendering/shader/Shader.h>
#include <engine/camera/CameraComponent.h>
#include <engine/util/functions.h>
#include <engine/rendering/architecture/RenderPass.h>
#include <engine/rendering/renderer/SimpleMeshRenderer.h>
#include "engine/rendering/voxelConeTracing/Globals.h"

class CameraComponent;
class MeshRenderer;

class ShadowMapPass : public RenderPass
{
public:
    ShadowMapPass(uint32_t shadowMapResolution);

    void update() override;

    GLuint getDepthTexture(int idx) const { return m_framebuffers[idx]->getDepthTexture(); }

    //GLuint getRenderTexture(int idx) const { return m_framebuffers[idx]->getRenderTexture(GL_COLOR_ATTACHMENT0); }

    void setProjSize(glm::vec2 projSize) { m_projectionSize = projSize; }

private:
    void render(glm::mat4 view, glm::mat4 proj) const;

private:
    std::unique_ptr<Framebuffer> m_framebuffers[MAX_DIR_LIGHT_COUNT];
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Shader> m_quadShader;

    std::shared_ptr<SimpleMeshRenderer> m_fullscreenQuadRenderer;

    glm::vec2 m_projectionSize{37.0f, 37.0f};

    uint32_t m_resolution;
};
