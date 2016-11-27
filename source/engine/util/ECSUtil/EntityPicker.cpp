#include "EntityPicker.h"
#include <engine/rendering/Screen.h>
#include <engine/util/Logger.h>
#include <engine/resource/ResourceManager.h>
#include "ECSUtil.h"
#include <engine/ecs/ECS.h>
#include "engine/util/colors.h"
#include <engine/rendering/voxelConeTracing/Globals.h>
#include "engine/camera/CameraComponent.h"

EntityPicker::EntityPicker()
{
    Input::subscribe(this);

    m_framebuffer = std::make_unique<Framebuffer>(Screen::getWidth(), Screen::getHeight());
    m_framebuffer->bind();

    auto depthTexture = std::make_shared<Texture2D>();
    depthTexture->create(Screen::getWidth(), Screen::getHeight(), GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);
    m_framebuffer->attachDepthBuffer(depthTexture);

    m_texture = std::make_shared<Texture2D>();
    m_texture->create(Screen::getWidth(), Screen::getHeight(), GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, Texture2DSettings::Custom);

    m_texture->bind();
    m_texture->setParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_texture->setParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_texture->setParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    m_texture->setParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    m_framebuffer->attachRenderTexture2D(m_texture, GL_COLOR_ATTACHMENT0);
    m_framebuffer->setDrawBuffers();
    m_framebuffer->checkFramebufferStatus();

    m_framebuffer->unbind();

    m_shader = ResourceManager::getShader("shaders/util/entityPicker.vert", "shaders/util/entityPicker.frag");
}

void EntityPicker::update() const
{
    m_framebuffer->begin(Screen::getWidth(), Screen::getHeight());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    m_shader->bind();
    m_shader->setCamera(MainCamera->view(), MainCamera->proj());
    ECSUtil::renderEntities(m_shader.get());

    m_framebuffer->end();
}

Entity EntityPicker::pick(int screenX, int screenY) const
{
    if (screenX >= Screen::getWidth() || screenY >= Screen::getHeight() || screenX < 0 || screenY < 0)
        return Entity();

    m_framebuffer->bind();
    RG_UINT32 info;
    glReadPixels(GLint(screenX), GLint(screenY), 1, 1, GL_RG_INTEGER, GL_UNSIGNED_INT, &info);
    m_framebuffer->unbind();

    // r = entity ID, g = entity version
    return ECS::getEntity(info.r, info.g);
}

void EntityPicker::onWindowEvent(const SDL_WindowEvent& windowEvent)
{
    switch (windowEvent.event)
    {
    case SDL_WINDOWEVENT_RESIZED:
        m_framebuffer->resize(static_cast<GLsizei>(windowEvent.data1), static_cast<GLsizei>(windowEvent.data2));
        break;
    default: break;
    }
}
