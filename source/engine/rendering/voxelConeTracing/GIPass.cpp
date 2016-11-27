#include "GIPass.h"
#include <vector>
#include <engine/rendering/Texture3D.h>
#include <engine/rendering/architecture/RenderPipeline.h>
#include <engine/rendering/util/GLUtil.h>
#include <engine/rendering/Screen.h>
#include "Globals.h"
#include <engine/rendering/lights/DirectionalLight.h>
#include <engine/geometry/Transform.h>
#include <engine/resource/ResourceManager.h>
#include <engine/rendering/renderer/MeshRenderers.h>
#include "settings/VoxelConeTracingSettings.h"
#include "VoxelRegion.h"
#include "engine/util/ECSUtil/ECSUtil.h"

#define DIRECT_LIGHTING_BIT 1
#define INDIRECT_DIFFUSE_LIGHTING_BIT 2
#define INDIRECT_SPECULAR_LIGHTING_BIT 4
#define AMBIENT_OCCLUSION_BIT 8

GIPass::GIPass()
    : RenderPass("GIPass")
{
    m_finalLightPassShader = ResourceManager::getShader("shaders/voxelConeTracing/finalLightingPass.vert", "shaders/voxelConeTracing/finalLightingPass.frag");
    m_fullscreenQuadRenderer = MeshRenderers::fullscreenQuad();
}

void GIPass::update()
{
    // Fetch the data
    Texture3D* voxelRadiance = m_renderPipeline->fetchPtr<Texture3D>("VoxelRadiance");
    auto clipRegions = m_renderPipeline->fetchPtr<std::vector<VoxelRegion>>("ClipRegions");
    auto camera = m_renderPipeline->getCamera();

    GLuint diffuseTexture = m_renderPipeline->fetch<GLuint>("DiffuseTexture");
    GLuint normalMap = m_renderPipeline->fetch<GLuint>("NormalMap");
    GLuint specularMap = m_renderPipeline->fetch<GLuint>("SpecularMap");
    GLuint emissionMap = m_renderPipeline->fetch<GLuint>("EmissionMap");
    GLuint depthTexture = m_renderPipeline->fetch<GLuint>("DepthTexture");

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GL::setViewport(Rect(0.f, 0.f, float(Screen::getWidth()), float(Screen::getHeight())));

    m_finalLightPassShader->bind();
    GLint textureUnit = 0;
    m_finalLightPassShader->bindTexture2D(diffuseTexture, "u_diffuseTexture", textureUnit++);
    m_finalLightPassShader->bindTexture2D(normalMap, "u_normalMap", textureUnit++);
    m_finalLightPassShader->bindTexture2D(specularMap, "u_specularMap", textureUnit++);
    m_finalLightPassShader->bindTexture2D(depthTexture, "u_depthTexture", textureUnit++);
    m_finalLightPassShader->bindTexture2D(emissionMap, "u_emissionMap", textureUnit++);
    m_finalLightPassShader->bindTexture3D(*voxelRadiance, "u_voxelRadiance", textureUnit++);

    m_finalLightPassShader->setInt("u_BRDFMode", RENDERING_SETTINGS.brdfMode);
    m_finalLightPassShader->setMatrix("u_viewProjInv", camera->viewProjInv());
    m_finalLightPassShader->setVector("u_volumeMin", clipRegions->at(0).getMinPosWorld());
    m_finalLightPassShader->setFloat("u_voxelSizeL0", clipRegions->at(0).voxelSize);
    m_finalLightPassShader->setVector("u_volumeCenterL0", clipRegions->at(0).getCenterPosWorld());
    m_finalLightPassShader->setUnsignedInt("u_volumeDimension", VOXEL_RESOLUTION);
    m_finalLightPassShader->setVector("u_eyePos", camera->getPosition());
    m_finalLightPassShader->setFloat("u_occlusionDecay", GI_SETTINGS.occlusionDecay);
    m_finalLightPassShader->setFloat("u_ambientOcclusionFactor", GI_SETTINGS.ambientOcclusionFactor);

    glm::vec3 volumeCenters[CLIP_REGION_COUNT];
    for (int i = 0; i < CLIP_REGION_COUNT; ++i)
        volumeCenters[i] = clipRegions->at(i).getCenterPosWorld();

    glUniform3fv(m_finalLightPassShader->getLocation("u_volumeCenters"), CLIP_REGION_COUNT, &volumeCenters[0][0]);

    // Set ShadowMap/Light uniforms
    ECSUtil::setDirectionalLightUniforms(m_finalLightPassShader.get(), textureUnit++);
    m_finalLightPassShader->setFloat("u_depthBias", SHADOW_SETTINGS.depthBias);
    m_finalLightPassShader->setFloat("u_usePoissonFilter", SHADOW_SETTINGS.usePoissonFilter ? 1.0f : 0.0f);

    m_finalLightPassShader->setFloat("u_traceStartOffset", GI_SETTINGS.traceStartOffset);
    m_finalLightPassShader->setFloat("u_stepFactor", GI_SETTINGS.stepFactor);

    int lightingMask = 0;
    lightingMask |= GI_SETTINGS.directLighting ? DIRECT_LIGHTING_BIT : 0;
    lightingMask |= GI_SETTINGS.indirectDiffuseLighting ? INDIRECT_DIFFUSE_LIGHTING_BIT : 0;
    lightingMask |= GI_SETTINGS.indirectSpecularLighting ? INDIRECT_SPECULAR_LIGHTING_BIT : 0;
    lightingMask |= GI_SETTINGS.ambientOcclusion ? AMBIENT_OCCLUSION_BIT : 0;

    m_finalLightPassShader->setInt("u_lightingMask", lightingMask);

    m_finalLightPassShader->setFloat("u_indirectDiffuseIntensity", GI_SETTINGS.indirectDiffuseIntensity);
    m_finalLightPassShader->setFloat("u_indirectSpecularIntensity", GI_SETTINGS.indirectSpecularIntensity);
    m_finalLightPassShader->setInt("u_visualizeMinLevelSelection", GI_SETTINGS.visualizeMinLevelSelection ? 1 : 0);

    m_fullscreenQuadRenderer->bindAndRender();

    glEnable(GL_DEPTH_TEST);
}
