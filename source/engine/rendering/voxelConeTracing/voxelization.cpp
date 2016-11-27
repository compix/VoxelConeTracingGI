#include "voxelization.h"
#include "engine/rendering/shader/Shader.h"
#include "engine/util/math.h"
#include "Globals.h"
#include "engine/geometry/BBox.h"
#include "engine/rendering/util/GLUtil.h"
#include "VoxelRegion.h"
#include "engine/util/ECSUtil/ECSUtil.h"

namespace voxelization
{
    int computeLowerBound(float value)
    {
        float f = std::floor(value);
        float c = std::ceil(value);

        // yields the correct integer represented by the  result of floor
        int result = static_cast<int>(f);

        // Floor and ceil can return the same value. Return a value that is 1 below in this case
        if (math::nearEq(f, c, math::EPSILON5))
            return result - 1;

        return result;
    }

    int computeUpperBound(float value)
    {
        float f = std::floor(value);
        float c = std::ceil(value);

        // yields the correct integer represented by the result of ceil
        int result = static_cast<int>(c);

        // Floor and ceil can return the same value. Return a value that is 1 above in this case
        if (math::nearEq(f, c, math::EPSILON5))
            return result + 1;

        return result;
    }

    glm::ivec3 computeLowerBound(const glm::vec3& vec)
    {
        return glm::ivec3(computeLowerBound(vec.x),
            computeLowerBound(vec.y),
            computeLowerBound(vec.z));
    }

    glm::ivec3 computeUpperBound(const glm::vec3& vec)
    {
        return glm::ivec3(computeUpperBound(vec.x),
            computeUpperBound(vec.y),
            computeUpperBound(vec.z));
    }

    void setViewProjectionMatrices(Shader* shader, const VoxelRegion& voxelRegion)
    {
        glm::vec3 size = voxelRegion.getExtentWorld();

        glm::mat4 proj[3];

        proj[0] = math::orthoLH(0.0f, size.z, 0.0f, size.y, 0.0f, size.x);
        proj[1] = math::orthoLH(0.0f, size.x, 0.0f, size.z, 0.0f, size.y);
        proj[2] = math::orthoLH(0.0f, size.x, 0.0f, size.y, 0.0f, size.z);

        glm::mat4 viewProj[3];
        glm::mat4 viewProjInv[3];

        glm::vec3 xyStart = voxelRegion.getMinPosWorld() + glm::vec3(0.0f, 0.0f, size.z);
        viewProj[0] = glm::lookAt(xyStart, xyStart + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        viewProj[1] = glm::lookAt(xyStart, xyStart + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        viewProj[2] = glm::lookAt(voxelRegion.getMinPosWorld(), voxelRegion.getMinPosWorld() + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        for (int i = 0; i < 3; ++i)
        {
            viewProj[i] = proj[i] * viewProj[i];
            viewProjInv[i] = glm::inverse(viewProj[i]);
        }

        glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), "u_viewProj"), 3, GL_FALSE, &viewProj[0][0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), "u_viewProjInv"), 3, GL_FALSE, &viewProjInv[0][0][0]);
    }

    void setViewports(Shader* shader, const glm::vec3& viewportSize)
    {
        glm::vec2 viewportSizes[3];
        viewportSizes[0] = glm::vec2(viewportSize.z, viewportSize.y);
        viewportSizes[1] = glm::vec2(viewportSize.x, viewportSize.z);
        viewportSizes[2] = glm::vec2(viewportSize.x, viewportSize.y);

        // Set the viewports
        glViewportIndexedf(0, 0.f, 0.f, viewportSizes[0].x, viewportSizes[0].y);
        glViewportIndexedf(1, 0.f, 0.f, viewportSizes[1].x, viewportSizes[1].y);
        glViewportIndexedf(2, 0.f, 0.f, viewportSizes[2].x, viewportSizes[2].y);

        glScissorIndexed(0, 0, 0, static_cast<GLsizei>(viewportSizes[0].x), static_cast<GLsizei>(viewportSizes[0].y));
        glScissorIndexed(1, 0, 0, static_cast<GLsizei>(viewportSizes[1].x), static_cast<GLsizei>(viewportSizes[1].y));
        glScissorIndexed(2, 0, 0, static_cast<GLsizei>(viewportSizes[2].x), static_cast<GLsizei>(viewportSizes[2].y));

        glUniform2fv(glGetUniformLocation(shader->getProgram(), "u_viewportSizes"), 3, &viewportSizes[0][0]);
    }
}

Voxelizer::Voxelizer()
{
    m_framebuffer = std::make_unique<Framebuffer>();
    m_framebuffer->bind();
    // Using ARB_framebuffer_no_attachments
    // Increased by 2 because revoxelization regions need to be extended by 1 in each direction to ensure that no fragments are missed
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, static_cast<GLint>(VOXEL_RESOLUTION + 2));
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, static_cast<GLint>(VOXEL_RESOLUTION + 2));
    //glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 8);
    //glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, GL_TRUE);
    m_framebuffer->checkFramebufferStatus();
    m_framebuffer->unbind();

    m_msaaFramebuffer = std::make_unique<Framebuffer>();
    m_msaaFramebuffer->bind();
    // Using ARB_framebuffer_no_attachments
    // Increased by 2 because revoxelization regions need to be extended by 1 in each direction to ensure that no fragments are missed
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, static_cast<GLint>(VOXEL_RESOLUTION + 2));
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, static_cast<GLint>(VOXEL_RESOLUTION + 2));
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 8);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, GL_TRUE);
    m_msaaFramebuffer->checkFramebufferStatus();
    m_msaaFramebuffer->unbind();
}

void Voxelizer::beginVoxelization(const VoxelizationDesc& desc)
{
    m_voxelizationDesc = desc;

    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    switch(desc.mode)
    {
    case VoxelizationMode::CONSERVATIVE:
        glDisable(GL_MULTISAMPLE);
        break;
    case VoxelizationMode::MSAA: 
        glEnable(GL_MULTISAMPLE);
        break;
    default: break;
    }

    Shader* shader = desc.voxelizationShader;

    getFramebuffer(desc.mode)->begin();
    shader->bind();

    shader->setInt("u_clipmapResolution", int(VOXEL_RESOLUTION));
    shader->setInt("u_clipmapResolutionWithBorder", int(VOXEL_RESOLUTION + 2));
}

void Voxelizer::voxelize(const VoxelRegion& voxelRegion, int clipmapLevel)
{
    Shader* shader = m_voxelizationDesc.voxelizationShader;
    auto& clipRegions = m_voxelizationDesc.clipRegions;

    // Extend by epsilon to prevent potential floating point imprecision problems (fragments that fail to be voxelized)
    glm::vec3 regionMinWorld = voxelRegion.getMinPosWorld() - math::EPSILON5;
    glm::vec3 regionMaxWorld = voxelRegion.getMaxPosWorld() + math::EPSILON5;

    // Use an extended Voxel Region for the viewProj matrix calculation to ensure that no pixels are missed
    VoxelRegion extendedRegion = voxelRegion;
    extendedRegion.extent = voxelRegion.extent + 2;
    extendedRegion.minPos -= 1;

    voxelization::setViewports(shader, extendedRegion.extent);
    voxelization::setViewProjectionMatrices(shader, extendedRegion);
    
    shader->setVector("u_regionMin", regionMinWorld);
    shader->setVector("u_regionMax", regionMaxWorld);
    shader->setInt("u_clipmapLevel", clipmapLevel);
    shader->setFloat("u_maxExtent", clipRegions[clipmapLevel].getExtentWorld().x);
    shader->setFloat("u_voxelSize", clipRegions[clipmapLevel].voxelSize);

    if (clipmapLevel > 0)
    {
        shader->setVector("u_prevRegionMin", clipRegions[clipmapLevel - 1].getMinPosWorld());
        shader->setVector("u_prevRegionMax", clipRegions[clipmapLevel - 1].getMaxPosWorld());
        shader->setFloat("u_downsampleTransitionRegionSize", m_voxelizationDesc.downsampleTransitionRegionSize * voxelRegion.voxelSize);
    }

    ECSUtil::renderEntitiesInAABB(BBox(regionMinWorld, regionMaxWorld), shader);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Voxelizer::endVoxelization(const Rect& originalViewport)
{
    getFramebuffer(m_voxelizationDesc.mode)->end();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    GL::setViewport(originalViewport);
    glEnable(GL_MULTISAMPLE);
}

Framebuffer* Voxelizer::getFramebuffer(VoxelizationMode mode)
{
    switch (mode)
    {
    case VoxelizationMode::CONSERVATIVE:
        return m_framebuffer.get();
    case VoxelizationMode::MSAA:
        return m_msaaFramebuffer.get();
    default: 
        assert(false);
        break;
    }

    return nullptr;
}
