#pragma once
#include <memory>
#include <engine/rendering/architecture/RenderPass.h>
#include <engine/rendering/shader/Shader.h>

class ForwardScenePass : public RenderPass
{
public:
    explicit ForwardScenePass();

    void update() override;

private:
    void render(bool wireframe) const;

private:
    std::shared_ptr<Shader> m_shader;
};
