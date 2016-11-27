#pragma once
#include <engine/rendering/architecture/RenderPass.h>
#include <engine/rendering/shader/Shader.h>
#include <engine/rendering/renderer/SimpleMeshRenderer.h>
#include <memory>

class GIPass : public RenderPass
{
public:
    GIPass();

    void update() override;

private:
    std::shared_ptr<Shader> m_finalLightPassShader;
    std::shared_ptr<SimpleMeshRenderer> m_fullscreenQuadRenderer;
};
