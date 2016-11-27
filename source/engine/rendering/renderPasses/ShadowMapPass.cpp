#include "ShadowMapPass.h"
#include <engine/util/Logger.h>
#include <engine/util/math.h>
#include <engine/rendering/Screen.h>
#include <engine/rendering/util/GLUtil.h>
#include <engine/geometry/Rect.h>
#include <engine/rendering/debug/DebugRenderer.h>
#include <engine/rendering/architecture/RenderPipeline.h>
#include <engine/resource/ResourceManager.h>
#include <engine/ecs/ECS.h>
#include <engine/rendering/lights/DirectionalLight.h>
#include <engine/rendering/renderer/MeshRenderers.h>
#include "engine/util/ECSUtil/ECSUtil.h"

ShadowMapPass::ShadowMapPass(uint32_t shadowMapResolution)
    : RenderPass("ShadowMapPass"), m_resolution(shadowMapResolution)
{
    for (int i = 0; i < MAX_DIR_LIGHT_COUNT; ++i)
    {
        m_framebuffers[i] = std::make_unique<Framebuffer>(shadowMapResolution, shadowMapResolution, false, GL_FLOAT, true);
        m_framebuffers[i]->bind();

        m_framebuffers[i]->checkFramebufferStatus();
        m_framebuffers[i]->unbind();
    }

    GL_ERROR_CHECK();

    m_fullscreenQuadRenderer = MeshRenderers::fullscreenQuad();
    m_quadShader = ResourceManager::getShader("shaders/simple/fullscreenQuad.vert", "shaders/simple/fullscreenQuad.frag", {"in_pos"});
    m_shader = ResourceManager::getShader("shaders/shadows/shadowMap.vert", "shaders/shadows/shadowMap.frag", {"in_pos"});
}

void ShadowMapPass::update()
{
    int lightCount = 0;
    for (auto dirLight : ECS::getEntitiesWithComponents<DirectionalLight, Transform>())
    {
        if (lightCount == MAX_DIR_LIGHT_COUNT)
            break;

        auto dirLightTransform = dirLight.getComponent<Transform>();
        auto dirLightComponent = dirLight.getComponent<DirectionalLight>();

        if (!dirLightComponent->shadowsEnabled)
        {
            lightCount++;
            continue;
        }

        m_framebuffers[lightCount]->bind();
        glDisable(GL_SCISSOR_TEST);

        glm::vec3 lightDir = dirLightTransform->getForward();
        glm::vec3 lightPos = dirLightTransform->getPosition();

        glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
        if (math::nearEq(std::abs(glm::dot(lightDir, up)), 1.0f))
            up = glm::vec3(0.0f, 0.0f, 1.0f);

        glm::mat4 view = glm::lookAt(lightPos, lightPos + lightDir, up);

        GL::setViewport(Rect(0.f, 0.f, static_cast<float>(m_resolution), static_cast<float>(m_resolution)));

        float hw = m_projectionSize.x * 0.5f;
        float hh = m_projectionSize.y * 0.5f;
        glm::mat4 proj = math::orthoLH(-hw, hw, -hh, hh, dirLightComponent->zNear, dirLightComponent->zFar);

        dirLightComponent->view = view;
        dirLightComponent->proj = proj;
        
        render(view, proj);

        dirLightComponent->shadowMap = getDepthTexture(lightCount);

        m_framebuffers[lightCount]->unbind();

        lightCount++;
    }

    GL::setViewport(Rect(0.0f, 0.0f, static_cast<float>(Screen::getWidth()), static_cast<float>(Screen::getHeight())));
}

void ShadowMapPass::render(glm::mat4 view, glm::mat4 proj) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->bind();
    m_shader->setMatrix("u_view", view);
    m_shader->setMatrix("u_proj", proj);

    ECSUtil::renderEntities(m_shader.get());
}
