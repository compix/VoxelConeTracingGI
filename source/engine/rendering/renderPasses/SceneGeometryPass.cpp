#include "SceneGeometryPass.h"
#include <engine/rendering/Screen.h>
#include <engine/camera/CameraComponent.h>
#include <engine/rendering/architecture/RenderPipeline.h>
#include <engine/ecs/ecs.h>
#include <engine/resource/ResourceManager.h>
#include "engine/util/ECSUtil/ECSUtil.h"
#include "engine/rendering/voxelConeTracing/settings/VoxelConeTracingSettings.h"

SceneGeometryPass::SceneGeometryPass()
    : RenderPass("SceneRenderPass")
{
    Input::subscribe(this);

    // Set up framebuffer
    m_framebuffer = std::make_unique<Framebuffer>(Screen::getWidth(), Screen::getHeight(), false);

    m_framebuffer->begin();

    // Albedo
    std::shared_ptr<Texture2D> albedo = std::make_shared<Texture2D>();
    albedo->create(Screen::getWidth(), Screen::getHeight(), GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
    m_framebuffer->attachRenderTexture2D(albedo, GL_COLOR_ATTACHMENT0);

    // Normal
    std::shared_ptr<Texture2D> normal = std::make_shared<Texture2D>();
    normal->create(Screen::getWidth(), Screen::getHeight(), GL_RGB16F, GL_RGB, GL_FLOAT);
    m_framebuffer->attachRenderTexture2D(normal, GL_COLOR_ATTACHMENT1);

    // Specular
    std::shared_ptr<Texture2D> specular = std::make_shared<Texture2D>();
    specular->create(Screen::getWidth(), Screen::getHeight(), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    m_framebuffer->attachRenderTexture2D(specular, GL_COLOR_ATTACHMENT2);

    // Emission
    std::shared_ptr<Texture2D> emission = std::make_shared<Texture2D>();
    emission->create(Screen::getWidth(), Screen::getHeight(), GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
    m_framebuffer->attachRenderTexture2D(emission, GL_COLOR_ATTACHMENT3);

    // Depth
    m_framebuffer->attachDepthBuffer(Screen::getWidth(), Screen::getHeight());
    m_framebuffer->setDrawBuffers();

    m_framebuffer->end();

    m_shader = ResourceManager::getShader("shaders/voxelConeTracing/scenePass.vert", "shaders/voxelConeTracing/scenePass.frag");
}

void SceneGeometryPass::render() const
{
    if (RENDERING_SETTINGS.wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    m_framebuffer->begin();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (RENDERING_SETTINGS.cullBackFaces)
    {
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    m_shader->bind();
    m_shader->setCamera(m_renderPipeline->getCamera()->view(), m_renderPipeline->getCamera()->proj());

    ECSUtil::renderEntities(m_shader.get());

    glDisable(GL_CULL_FACE);
    m_framebuffer->end();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SceneGeometryPass::onWindowEvent(const SDL_WindowEvent& windowEvent)
{
    switch (windowEvent.event)
    {
    case SDL_WINDOWEVENT_RESIZED:
        m_framebuffer->resize(static_cast<GLsizei>(windowEvent.data1), static_cast<GLsizei>(windowEvent.data2));
        break;
    default: break;
    }
}

void SceneGeometryPass::update()
{
    render();

    m_renderPipeline->put<GLuint>("DiffuseTexture", getRenderTexture(GL_COLOR_ATTACHMENT0));
    m_renderPipeline->put<GLuint>("NormalMap", getRenderTexture(GL_COLOR_ATTACHMENT1));
    m_renderPipeline->put<GLuint>("SpecularMap", getRenderTexture(GL_COLOR_ATTACHMENT2));
    m_renderPipeline->put<GLuint>("EmissionMap", getRenderTexture(GL_COLOR_ATTACHMENT3));
    m_renderPipeline->put<GLuint>("DepthTexture", getDepthTexture());
}
