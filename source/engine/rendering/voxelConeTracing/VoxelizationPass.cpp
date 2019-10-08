#include "VoxelizationPass.h"
#include <GL/glew.h>
#include <engine/rendering/Texture3D.h>
#include <engine/util/Logger.h>
#include <engine/geometry/BBox.h>
#include <engine/rendering/architecture/RenderPipeline.h>
#include <engine/ecs/ECS.h>
#include <engine/resource/ResourceManager.h>
#include <engine/rendering/util/ImageCleaner.h>
#include <engine/rendering/renderer/MeshRenderer.h>
#include "settings/VoxelConeTracingSettings.h"
#include "voxelization.h"
#include "Downsampler.h"
#include "engine/util/QueryManager.h"
#include "VoxelConeTracing.h"

VoxelizationPass::VoxelizationPass()
    : RenderPass("VoxelizationPass")
{
    m_voxelizeShader = ResourceManager::getShader("shaders/voxelConeTracing/conservative6SeparatingOpacityVoxelization.vert", 
    	"shaders/voxelConeTracing/conservative6SeparatingOpacityVoxelization.frag", "shaders/voxelConeTracing/conservative6SeparatingOpacityVoxelization.geom");

    m_clipRegions.resize(CLIP_REGION_COUNT);
}

void VoxelizationPass::init(float extentWorldLevel0)
{
    int extent = VOXEL_RESOLUTION;
    int halfExtent = extent / 2;

    // Define clip regions centered around the origin (0, 0, 0) in voxel coordinates
    for (size_t i = 0; i < m_clipRegions.size(); ++i)
    {
        m_clipRegions[i].minPos = glm::ivec3(-halfExtent);
        m_clipRegions[i].extent = glm::ivec3(extent);
        m_clipRegions[i].voxelSize = (extentWorldLevel0 * std::exp2f(static_cast<float>(i))) / extent;
    }

    // Move regions to be "centered" (close to the center in discrete voxel coordinates) around the camera
    auto clipRegionBBoxes = m_renderPipeline->fetchPtr<std::vector<BBox>>("ClipRegionBBoxes");
    
    for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_REGION_COUNT; ++clipmapLevel)
    {
        auto& clipRegion = m_clipRegions[clipmapLevel];
    
        // Compute the closest delta in voxel coordinates
        glm::ivec3 delta = computeChangeDeltaV(clipmapLevel, clipRegionBBoxes->at(clipmapLevel));
    
        clipRegion.minPos += delta;
    }

    m_forceFullRevoxelization = true;
}

void VoxelizationPass::update()
{
    // Fetch the data
    m_voxelOpacity = m_renderPipeline->fetchPtr<Texture3D>("VoxelOpacity");
    auto clipRegionBBoxes = m_renderPipeline->fetchPtr<std::vector<BBox>>("ClipRegionBBoxes");

    if (m_forceFullRevoxelization)
    {
        for (uint32_t i = 0; i < CLIP_REGION_COUNT; ++i)
        {
            m_revoxelizationRegions[i].clear();
            m_revoxelizationRegions[i].push_back(m_clipRegions[i]);
        }

        m_forceFullRevoxelization = false;
    }
    else
    {
        for (uint32_t i = 0; i < CLIP_REGION_COUNT; ++i)
        {
            m_revoxelizationRegions[i].clear();
            computeRevoxelizationRegionsClipmap(i, clipRegionBBoxes->at(i));
        }

        computeRevoxelizationRegionsDynamicEntities();
    }

    QueryManager::beginElapsedTime(QueryTarget::GPU, "Clear Voxel Opacity Regions");
    for (uint32_t i = 0; i < CLIP_REGION_COUNT; ++i)
    {
        // Clear the regions
        for (auto& region : m_revoxelizationRegions[i])
        {
            ImageCleaner::clear6FacesImage3D(*m_voxelOpacity, GL_RGBA8, region.getMinPosImage(m_clipRegions[i].extent), region.extent, VOXEL_RESOLUTION, GLuint(i), 1);
        }
    }

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    QueryManager::endElapsedTime(QueryTarget::GPU, "Clear Voxel Opacity Regions");

    VoxelizationDesc desc;
    desc.mode = VoxelizationMode::CONSERVATIVE;
    desc.clipRegions = m_clipRegions;
    desc.voxelizationShader = m_voxelizeShader.get();
    desc.downsampleTransitionRegionSize = GI_SETTINGS.downsampleTransitionRegionSize;
    Voxelizer* voxelizer = VoxelConeTracing::voxelizer();
    voxelizer->beginVoxelization(desc);
    m_voxelizeShader->bindImage3D(*m_voxelOpacity, "u_voxelOpacity", GL_WRITE_ONLY, GL_RGBA8, 0); // GL_WRITE_ONLY, GL_RGBA8

    for (int i = 0; i < CLIP_REGION_COUNT; ++i)
    {
        // Voxelize the regions
        for (auto& region : m_revoxelizationRegions[i])
        {
            voxelizer->voxelize(region, i);
        }
    }

    voxelizer->endVoxelization(m_renderPipeline->getCamera()->getViewport());
    
    bool m_downsampleUpdateNecessary = false;

    // Downsample - this can be significantly improved by downsampling only the required regions
    for (int i = 1; i < CLIP_REGION_COUNT; ++i)
    {
        if (m_downsampleUpdateNecessary || m_revoxelizationRegions[i].size() > 0)
        {
            m_downsampleUpdateNecessary = true;
            Downsampler::downsampleOpacity(m_voxelOpacity, &m_clipRegions, i);
        }
    }

    recordDebugInfo();

    m_renderPipeline->putPtr("ClipRegions", &m_clipRegions);
}

void VoxelizationPass::computeRevoxelizationRegionsClipmap(uint32_t clipmapLevel, const BBox& curBBox)
{
    auto& clipRegion = m_clipRegions[clipmapLevel];
    float voxelSize = clipRegion.voxelSize;
    glm::ivec3 extent = clipRegion.extent;

    // Compute the closest delta in voxel coordinates
    glm::ivec3 delta = computeChangeDeltaV(clipmapLevel, curBBox);
    glm::ivec3 absDelta = glm::abs(delta);

    clipRegion.minPos += delta;
    
    // If the new region is outside of the previous region then a full revoxelization of the new region is required.
    if (absDelta.x >= extent.x || absDelta.y >= extent.y || absDelta.z >= extent.z)
    {
        m_revoxelizationRegions[clipmapLevel].push_back(clipRegion);
        return;
    }

    glm::ivec3 newMin = clipRegion.minPos;
    glm::ivec3 newMax = clipRegion.getMaxPos();

    // Compute the regions for revoxelization
    if (absDelta.x >= m_minChange[clipmapLevel])
    {
        auto regionExtent = glm::ivec3(absDelta.x, extent.y, extent.z);

        if (delta.x < 0)
            m_revoxelizationRegions[clipmapLevel].push_back(VoxelRegion(newMin, regionExtent, voxelSize));
        else
            m_revoxelizationRegions[clipmapLevel].push_back(VoxelRegion(newMax - regionExtent, regionExtent, voxelSize));
    }

    if (absDelta.y >= m_minChange[clipmapLevel])
    {
        auto regionExtent = glm::ivec3(extent.x, absDelta.y, extent.z);

        if (delta.y < 0)
            m_revoxelizationRegions[clipmapLevel].push_back(VoxelRegion(newMin, regionExtent, voxelSize));
        else
            m_revoxelizationRegions[clipmapLevel].push_back(VoxelRegion(newMax - regionExtent, regionExtent, voxelSize));
    }

    if (absDelta.z >= m_minChange[clipmapLevel])
    {
        auto regionExtent = glm::ivec3(extent.x, extent.y, absDelta.z);

        if (delta.z < 0)
            m_revoxelizationRegions[clipmapLevel].push_back(VoxelRegion(newMin, regionExtent, voxelSize));
        else
            m_revoxelizationRegions[clipmapLevel].push_back(VoxelRegion(newMax - regionExtent, regionExtent, voxelSize));
    }
}

glm::ivec3 VoxelizationPass::computeChangeDeltaV(uint32_t clipmapLevel, const BBox& cameraRegionBBox)
{
    const auto& clipRegion = m_clipRegions[clipmapLevel];
    float voxelSize = clipRegion.voxelSize;

    glm::vec3 deltaW = cameraRegionBBox.min() - clipRegion.getMinPosWorld();

    // The camera needs to move at least the specified minChange amount for the portion to be revoxelized
    float minChange = voxelSize * m_minChange[clipmapLevel];

    // Compute the closest delta in voxel coordinates
    glm::ivec3 delta = glm::ivec3(glm::trunc(deltaW / minChange)) * m_minChange[clipmapLevel];

    return delta;
}

void VoxelizationPass::computeRevoxelizationRegionsDynamicEntities()
{
    // Revoxelize portions of entities that moved
    m_portionsOfDynamicEntities.clear();
    std::vector<bool> markedPortions;

    for (auto e : ECS::getEntitiesWithComponents<Transform, MeshRenderer>())
    {
        auto transform = e.getComponent<Transform>();
        if (transform->hasChangedSinceLastFrame())
        {
            BBox bbox = transform->getBBox();
            auto& lastFrameBBox = transform->getLastFrameBBox();
            if (bbox.overlaps(lastFrameBBox))
                bbox.unite(transform->getLastFrameBBox());
            else
                m_portionsOfDynamicEntities.push_back(lastFrameBBox);

            m_portionsOfDynamicEntities.push_back(bbox);
        }
    }

    for (auto& e : m_activatedDeactivatedEntities)
    {
        auto transform = e.getComponent<Transform>();
        BBox bbox = transform->getBBox();
        auto& lastFrameBBox = transform->getLastFrameBBox();
        if (bbox.overlaps(lastFrameBBox))
            bbox.unite(transform->getLastFrameBBox());
        else
            m_portionsOfDynamicEntities.push_back(lastFrameBBox);

        m_portionsOfDynamicEntities.push_back(bbox);
    }

    m_activatedDeactivatedEntities.clear();

    if (m_portionsOfDynamicEntities.size() == 0)
        return;

    // Marked portions will be skipped
    markedPortions.resize(m_portionsOfDynamicEntities.size(), false);

    // Unite overlapping portions
    for (size_t i = 0; i < m_portionsOfDynamicEntities.size(); ++i)
    {
        if (markedPortions[i])
            continue;

        BBox& a = m_portionsOfDynamicEntities[i];

        size_t j = i + 1;
        while (j < m_portionsOfDynamicEntities.size())
        {
            BBox& b = m_portionsOfDynamicEntities[j];
            if (!markedPortions[j] && a.overlaps(b))
            {
                a.unite(b);
                markedPortions[j] = true;
                j = i + 1;
            }
            else
                ++j;
        }
    }

    // Remove all marked portions
    for (int i = int(m_portionsOfDynamicEntities.size()) - 1; i >= 0; --i)
    {
        if (markedPortions[i])
            m_portionsOfDynamicEntities.erase(m_portionsOfDynamicEntities.begin() + i);
    }

    // Compute the correct regions for each clip region and add for revoxelization
    for (size_t j = 0; j < m_portionsOfDynamicEntities.size(); ++j)
    {
        const BBox& portion = m_portionsOfDynamicEntities[j];

        for (uint32_t i = 0; i < CLIP_REGION_COUNT; ++i)
        {
            auto& clipRegion = m_clipRegions[i];

            VoxelRegion region;
            region.voxelSize = clipRegion.voxelSize;

            // Geometry can be very thin (0 scale) and yield an extent of 0 so it's important to make
            // it a bit thicker (BBox) to avoid this problem (extend by epsilon in each direction). 
            // Just setting extent to 1 in this case isn't enough either because if this thin 
            // geometry is exactly at a discrete voxel size step/is right between two voxels (voxel coords -> world coords)
            // the extent should be two because it falls in both voxels in this case.

            glm::vec3 pMin = portion.min() / region.voxelSize - math::EPSILON5;
            glm::vec3 pMax = portion.max() / region.voxelSize + math::EPSILON5;

            region.minPos = voxelization::computeLowerBound(pMin) - m_overestimationWidth;
            glm::ivec3 maxPos = voxelization::computeUpperBound(pMax) + m_overestimationWidth;
            region.extent = maxPos - region.minPos;

            // Skip if the region is outside of the current clipmap
            if (glm::any(glm::greaterThanEqual(region.minPos, clipRegion.getMaxPos())) || glm::any(glm::lessThanEqual(region.getMaxPos(), clipRegion.minPos)))
                continue;

            // Clamp region to the current clipRegion
            maxPos = glm::min(region.getMaxPos(), clipRegion.getMaxPos());
            region.minPos = glm::max(region.minPos, clipRegion.minPos);
            region.extent = maxPos - region.minPos;

            m_revoxelizationRegions[i].push_back(region);
        }
    }
}

void VoxelizationPass::receive(const EntityDeactivatedEvent& e)
{
    if (e.entity.hasComponents<Transform, MeshRenderer>())
        m_activatedDeactivatedEntities.push_back(e.entity);
}

void VoxelizationPass::receive(const EntityActivatedEvent& e)
{
    if (e.entity.hasComponents<Transform, MeshRenderer>())
        m_activatedDeactivatedEntities.push_back(e.entity);
}

void VoxelizationPass::recordDebugInfo()
{
    for (int i = 0; i < CLIP_REGION_COUNT; ++i)
    {
        if (m_revoxelizationRegions[i].size() > 0)
        {
            m_debugInfo.lastRevoxelizationRegions.clear();
            break;
        }
    }

    for (int i = 0; i < CLIP_REGION_COUNT; ++i)
    {
        if (m_revoxelizationRegions[i].size() > 0)
            m_debugInfo.lastRevoxelizationRegions.insert(m_debugInfo.lastRevoxelizationRegions.end(), 
                m_revoxelizationRegions[i].begin(), m_revoxelizationRegions[i].end());
    }
}
