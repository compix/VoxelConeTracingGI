#include "VoxelConeTracing.h"
#include "Downsampler.h"
#include "engine/rendering/util/ImageCleaner.h"

Voxelizer* VoxelConeTracing::m_voxelizer = nullptr;

void VoxelConeTracing::init()
{
    ImageCleaner::init();
    Downsampler::init();

    m_voxelizer = new Voxelizer();
}

void VoxelConeTracing::terminate()
{
    if (m_voxelizer)
        delete m_voxelizer;
}
