#include "ForwardScenePass.h"
#include <gl/glew.h>
#include <engine/resource/ResourceManager.h>
#include <engine/rendering/architecture/RenderPipeline.h>
#include <engine/util/functions.h>
#include "engine/rendering/voxelConeTracing/Globals.h"
#include "engine/rendering/voxelConeTracing/settings/VoxelConeTracingSettings.h"
#include "engine/util/ECSUtil/ECSUtil.h"

ForwardScenePass::ForwardScenePass()
    : RenderPass("ForwardScenePass")
{
    m_shader = ResourceManager::getShader("shaders/forwardShadingPass.vert", "shaders/forwardShadingPass.frag");
}

void ForwardScenePass::update()
{
    render(false);

    if (RENDERING_SETTINGS.wireFrame)
        render(true);
}

void ForwardScenePass::render(bool wireframe) const
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    if (wireframe)
    {
        //glLineWidth(5.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

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
    m_shader->setFloat("u_wireframe", wireframe ? 1.0f : -1.0f);
    m_shader->setVector("u_eyePos", MainCamera->getPosition());
    m_shader->setVector("u_color", glm::vec4(1.0f));

    if (wireframe)
        m_shader->setVector("u_color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

    // Set ShadowMap/Light uniforms
    GLint shadowMapStartTextureUnit = 4;
    ECSUtil::setDirectionalLightUniforms(m_shader.get(), shadowMapStartTextureUnit);
    m_shader->setFloat("u_depthBias", SHADOW_SETTINGS.depthBias);
    m_shader->setFloat("u_usePoissonFilter", SHADOW_SETTINGS.usePoissonFilter ? 1.0f : 0.0f);

    m_shader->setInt("u_BRDFMode", RENDERING_SETTINGS.brdfMode);
    m_shader->setCamera(MainCamera->view(), MainCamera->proj());

    ECSUtil::renderEntities(m_shader.get());

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
}
