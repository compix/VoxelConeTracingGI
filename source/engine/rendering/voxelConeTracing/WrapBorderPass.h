#pragma once
#include <engine/rendering/shader/Shader.h>
#include <engine/rendering/architecture/RenderPass.h>
#include <memory>

class MeshRenderer;
class Texture3D;
class BBox;

class WrapBorderPass : public RenderPass
{
public:
    WrapBorderPass();

    void update() override;

private:
    void copyBorder(Texture3D* texture) const;

private:
    std::shared_ptr<Shader> m_shader;
};
