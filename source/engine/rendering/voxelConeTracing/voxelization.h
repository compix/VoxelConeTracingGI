#pragma once
#include <glm/glm.hpp>
#include "engine/rendering/Framebuffer.h"

class Rect;
class Shader;
struct VoxelRegion;

enum class VoxelizationMode
{
    CONSERVATIVE,
    MSAA
};

struct VoxelizationDesc
{
    Shader* voxelizationShader{nullptr};
    VoxelizationMode mode{VoxelizationMode::CONSERVATIVE};
    std::vector<VoxelRegion> clipRegions;
    int downsampleTransitionRegionSize{0};
};

class Voxelizer
{
public:
    Voxelizer();

    void beginVoxelization(const VoxelizationDesc& desc);

    void voxelize(const VoxelRegion& voxelRegion, int clipmapLevel);

    void endVoxelization(const Rect& originalViewport);

private:
    Framebuffer* getFramebuffer(VoxelizationMode mode);

private:
    VoxelizationDesc m_voxelizationDesc;
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<Framebuffer> m_msaaFramebuffer;
};

namespace voxelization
{
    int computeLowerBound(float value);
    int computeUpperBound(float value);
    glm::ivec3 computeLowerBound(const glm::vec3& vec);
    glm::ivec3 computeUpperBound(const glm::vec3& vec);

    void setViewProjectionMatrices(Shader* shader, const VoxelRegion& voxelRegion);
    void setViewports(Shader* shader, const glm::vec3& viewportSize);
}
