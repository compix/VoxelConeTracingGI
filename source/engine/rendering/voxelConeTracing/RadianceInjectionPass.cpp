#include "RadianceInjectionPass.h"
#include <GL/glew.h>
#include <engine/camera/CameraComponent.h>
#include <engine/rendering/Texture3D.h>
#include <engine/rendering/architecture/RenderPipeline.h>
#include <engine/resource/ResourceManager.h>
#include <engine/ecs/ECS.h>
#include "settings/VoxelConeTracingSettings.h"
#include "voxelization.h"
#include "Downsampler.h"
#include "engine/util/QueryManager.h"
#include "VoxelConeTracing.h"
#include "engine/rendering/util/ImageCleaner.h"
#include "engine/util/ECSUtil/ECSUtil.h"
#include "ClipmapUpdatePolicy.h"

RadianceInjectionPass::RadianceInjectionPass()
    : RenderPass("RadianceInjectionPass")
{
    m_conservativeVoxelizationShader = ResourceManager::getShader("shaders/voxelConeTracing/injectLightByConservativeVoxelization.vert",
        "shaders/voxelConeTracing/injectLightByConservativeVoxelization.frag", "shaders/voxelConeTracing/injectLightByConservativeVoxelization.geom");

    m_msaaVoxelizationShader = ResourceManager::getShader("shaders/voxelConeTracing/injectLightByMSAAVoxelization.vert",
        "shaders/voxelConeTracing/injectLightByMSAAVoxelization.frag", "shaders/voxelConeTracing/injectLightByMSAAVoxelization.geom");

    m_copyAlphaShader = ResourceManager::getComputeShader("shaders/voxelConeTracing/copyAlpha6Faces.comp");

    m_cachedClipRegions.resize(CLIP_REGION_COUNT);
}

void RadianceInjectionPass::update()
{
    auto voxelRadiance = m_renderPipeline->fetchPtr<Texture3D>("VoxelRadiance");
    auto voxelOpacity = m_renderPipeline->fetchPtr<Texture3D>("VoxelOpacity");
    m_clipmapUpdatePolicy = m_renderPipeline->fetchPtr<ClipmapUpdatePolicy>("ClipmapUpdatePolicy");

    auto clipRegions = m_renderPipeline->fetchPtr<std::vector<VoxelRegion>>("ClipRegions");

    if (m_initializing)
    {
        m_cachedClipRegions = *clipRegions;
    }
    else
    {
        auto& levelsToUpdate = m_clipmapUpdatePolicy->getLevelsScheduledForUpdate();
        for (auto level : levelsToUpdate)
        {
            m_cachedClipRegions[level] = clipRegions->at(level);
        }
    }

    injectByVoxelization(getSelectedShader(), voxelRadiance, m_voxelizationMode);
    copyAlpha(voxelRadiance, voxelOpacity);
    downsample(voxelRadiance);

    m_initializing = false;
}

void RadianceInjectionPass::injectByVoxelization(Shader* shader, Texture3D* voxelRadiance, VoxelizationMode voxelizationMode)
{
    static unsigned char zero[]{ 0, 0, 0, 0 };

    QueryManager::beginElapsedTime(QueryTarget::GPU, "Clear Radiance Voxels");

    auto& levelsToUpdate = m_clipmapUpdatePolicy->getLevelsScheduledForUpdate();

    if (m_clipmapUpdatePolicy->getType() == ClipmapUpdatePolicy::Type::ALL_PER_FRAME)
    {
        glClearTexImage(*voxelRadiance, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
    }
    else
    {
        for (auto level : levelsToUpdate)
        {
            auto clipLevel = static_cast<GLuint>(level);
            ImageCleaner::clear6FacesImage3D(*voxelRadiance, GL_RGBA8, glm::ivec3(0), glm::ivec3(VOXEL_RESOLUTION), VOXEL_RESOLUTION, clipLevel, 1);
        }

        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    QueryManager::endElapsedTime(QueryTarget::GPU, "Clear Radiance Voxels");

    QueryManager::beginElapsedTime(QueryTarget::GPU, "Radiance Voxelization");
    VoxelizationDesc desc;
    desc.mode = voxelizationMode;
    desc.clipRegions = m_cachedClipRegions;
    desc.voxelizationShader = shader;
    desc.downsampleTransitionRegionSize = GI_SETTINGS.downsampleTransitionRegionSize;
    Voxelizer* voxelizer = VoxelConeTracing::voxelizer();
    voxelizer->beginVoxelization(desc);
    shader->bindImage3D(*voxelRadiance, "u_voxelRadiance", GL_READ_WRITE, GL_R32UI, 0);

    // Set ShadowMap/Light uniforms
    GLint shadowMapStartTextureUnit = 5;
    ECSUtil::setDirectionalLightUniforms(shader, shadowMapStartTextureUnit, SHADOW_SETTINGS.radianceVoxelizationPCFRadius);

    shader->setFloat("u_depthBias", SHADOW_SETTINGS.depthBias);
    shader->setFloat("u_usePoissonFilter", SHADOW_SETTINGS.usePoissonFilter ? 1.0f : 0.0f);

    for (auto level : levelsToUpdate)
    {
        voxelizer->voxelize(m_cachedClipRegions.at(level), level);
    }

    voxelizer->endVoxelization(m_renderPipeline->getCamera()->getViewport());
    QueryManager::endElapsedTime(QueryTarget::GPU, "Radiance Voxelization");
}

void RadianceInjectionPass::copyAlpha(Texture3D* voxelRadiance, Texture3D* voxelOpacity, int clipLevel) const
{
    m_copyAlphaShader->bind();
    m_copyAlphaShader->bindTexture3D(*voxelOpacity, "u_srcTexture", 0);
    m_copyAlphaShader->bindImage3D(*voxelRadiance, "u_dstImage", GL_READ_WRITE, GL_RGBA8, 0);

    m_copyAlphaShader->setInt("u_clipmapResolution", VOXEL_RESOLUTION);
    m_copyAlphaShader->setInt("u_clipmapResolutionWithBorder", VOXEL_RESOLUTION + 2);
    m_copyAlphaShader->setInt("u_clipmapLevel", clipLevel);

    GLuint groupCount = VOXEL_RESOLUTION / 8;
    m_copyAlphaShader->dispatchCompute(groupCount, groupCount, groupCount);
}

Shader* RadianceInjectionPass::getSelectedShader()
{
    switch (GI_SETTINGS.radianceInjectionMode)
    {
    case 0:
        m_voxelizationMode = VoxelizationMode::CONSERVATIVE;
        return m_conservativeVoxelizationShader.get();
    case 1:
        m_voxelizationMode = VoxelizationMode::MSAA;
        return m_msaaVoxelizationShader.get();
    default:
        assert(false);
        return nullptr;
    }
}

void RadianceInjectionPass::downsample(Texture3D* voxelRadiance) const
{
    QueryManager::beginElapsedTime(QueryTarget::GPU, "Radiance Downsampling");
    auto& levelsToUpdate = m_clipmapUpdatePolicy->getLevelsScheduledForUpdate();

    if (m_clipmapUpdatePolicy->getType() == ClipmapUpdatePolicy::Type::ALL_PER_FRAME)
    {
        Downsampler::downsample(voxelRadiance, &m_cachedClipRegions);
    }
    else
    {
        for (auto level : levelsToUpdate)
        {
            if (level > 0)
                Downsampler::downsample(voxelRadiance, &m_cachedClipRegions, level);
        }
    }

    QueryManager::endElapsedTime(QueryTarget::GPU, "Radiance Downsampling");
}

void RadianceInjectionPass::copyAlpha(Texture3D* voxelRadiance, Texture3D* voxelOpacity) const
{
    QueryManager::beginElapsedTime(QueryTarget::GPU, "Copy Alpha");
    auto& levelsToUpdate = m_clipmapUpdatePolicy->getLevelsScheduledForUpdate();

    // Copy alpha from opacity texture (this allows us to access just one texture during GI pass)
    for (auto level : levelsToUpdate)
    {
        copyAlpha(voxelRadiance, voxelOpacity, level);
    }

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    QueryManager::endElapsedTime(QueryTarget::GPU, "Copy Alpha");
}
