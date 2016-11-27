#pragma once
#include <engine/rendering/shader/Shader.h>
#include <engine/rendering/geometry/Mesh.h>
#include <engine/rendering/architecture/RenderPass.h>
#include "voxelization.h"
#include "Globals.h"
#include "ClipmapUpdatePolicy.h"

class Framebuffer;
class Texture3D;
class BBox;

struct DirLight
{
    glm::vec3 position;
    glm::vec3 direction;

    DirLight(const glm::vec3& pos, const glm::vec3& dir)
        : position(pos), direction(dir) {}

    DirLight() {}
};

class RadianceInjectionPass : public RenderPass
{
public:
    explicit RadianceInjectionPass();

    void update() override;
private:
    void injectByVoxelization(Shader* shader, Texture3D* voxelRadiance, VoxelizationMode voxelizationMode);

    void downsample(Texture3D* voxelRadiance) const;
    void copyAlpha(Texture3D* voxelRadiance, Texture3D* voxelOpacity) const;
    void copyAlpha(Texture3D* voxelRadiance, Texture3D* voxelOpacity, int clipLevel) const;
    Shader* getSelectedShader();
private:
    std::shared_ptr<Shader> m_conservativeVoxelizationShader;
    std::shared_ptr<Shader> m_msaaVoxelizationShader;
    std::shared_ptr<Shader> m_copyAlphaShader;
    VoxelizationMode m_voxelizationMode{VoxelizationMode::CONSERVATIVE};

    ClipmapUpdatePolicy* m_clipmapUpdatePolicy{ nullptr };
    std::vector<VoxelRegion> m_cachedClipRegions;
    bool m_initializing{ true };
};
