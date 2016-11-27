#include "WrapBorderPass.h"
#include "engine/rendering/architecture/RenderPipeline.h"
#include "engine/resource/ResourceManager.h"
#include "engine/rendering/Texture3D.h"
#include "Globals.h"

WrapBorderPass::WrapBorderPass()
    : RenderPass("WrapBorderPass")
{
    m_shader = ResourceManager::getComputeShader("shaders/voxelConeTracing/copyWrappedBorder.comp");
}

void WrapBorderPass::update()
{
    // Fetch the data
    auto voxelOpacity = m_renderPipeline->fetchPtr<Texture3D>("VoxelOpacity");
    auto voxelRadiance = m_renderPipeline->fetchPtr<Texture3D>("VoxelRadiance");

    copyBorder(voxelOpacity);
    copyBorder(voxelRadiance);
}

void WrapBorderPass::copyBorder(Texture3D* texture) const
{
    m_shader->bind();

    m_shader->setInt("u_clipmapResolution", VOXEL_RESOLUTION);
    m_shader->setInt("u_clipmapResolutionWithBorder", VOXEL_RESOLUTION + 2);
    m_shader->setInt("u_faceCount", FACE_COUNT);
    m_shader->setInt("u_clipmapCount", CLIP_REGION_COUNT);
    m_shader->bindImage3D(*texture, "u_image", GL_READ_WRITE, GL_RGBA8, 0);

    float borderWidth2 = 2.0f;
    GLuint groupCount = GLuint(ceil((VOXEL_RESOLUTION + borderWidth2) / 8.0f));
    m_shader->dispatchCompute(groupCount, groupCount, groupCount);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
