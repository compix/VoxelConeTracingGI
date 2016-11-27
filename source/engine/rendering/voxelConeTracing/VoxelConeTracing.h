#pragma once
#include "voxelization.h"

class VoxelConeTracing
{
public:
    static void init();
    static void terminate();

    static Voxelizer* voxelizer() { return m_voxelizer; }

private:
    static Voxelizer* m_voxelizer;
};
