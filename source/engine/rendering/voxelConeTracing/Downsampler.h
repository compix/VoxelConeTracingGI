#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include "VoxelRegion.h"

class Texture3D;
class Shader;

class Downsampler
{
public:
    static void init();

    static void downsampleOpacity(Texture3D* texture, const std::vector<VoxelRegion>* clipRegions, int clipmapLevel);
    static void downsample(Texture3D* image, const std::vector<VoxelRegion>* clipRegions);
    static void downsample(Texture3D* image, const std::vector<VoxelRegion>* clipRegions, int clipmapLevel);

private:
    static std::shared_ptr<Shader> m_downsampleOpacityShader;
    static std::shared_ptr<Shader> m_downsampleShader;
};
