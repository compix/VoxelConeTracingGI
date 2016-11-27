#pragma once
#include <memory>
#include <engine/rendering/Framebuffer.h>
#include <engine/rendering/shader/Shader.h>
#include "engine/input/Input.h"

class Entity;

class EntityPicker : public InputHandler
{
public:
    EntityPicker();

    void update() const;
    Entity pick(int screenX, int screenY) const;

protected:
    void onWindowEvent(const SDL_WindowEvent& windowEvent) override;

private:
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::shared_ptr<Shader> m_shader;

    std::shared_ptr<Texture2D> m_texture;
};
