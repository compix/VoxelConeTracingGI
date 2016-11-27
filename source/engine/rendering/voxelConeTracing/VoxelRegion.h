#pragma once
#include <glm/glm.hpp>

struct VoxelRegion
{
    VoxelRegion() {}
    VoxelRegion(const glm::ivec3& minPos, const glm::ivec3& extent, float voxelSize)
        :minPos(minPos), extent(extent), voxelSize(voxelSize) {}

    /**
     * Returns the minimum position of the clip region in world coordinates.
     */
    glm::vec3 getMinPosWorld() const { return glm::vec3(minPos) * voxelSize; }

    /**
    * Returns the maximum position of the clip region in world coordinates.
    */
    glm::vec3 getMaxPosWorld() const { return glm::vec3(getMaxPos()) * voxelSize; }

    /**
    * Returns the minimum position of the clip region in image coordinates using toroidal addressing.
    * Note: The % operator is expected to be defined by the C++11 standard.
    */
    glm::ivec3 getMinPosImage(const glm::ivec3& imageSize) const { return ((minPos % imageSize) + imageSize) % imageSize; }

    /**
    * Returns the maximum position of the clip region in image coordinates using toroidal addressing.
    * Note: The % operator is expected to be defined by the C++11 standard.
    */
    glm::ivec3 getMaxPosImage(const glm::ivec3& imageSize) const { return ((getMaxPos() % imageSize) + imageSize) % imageSize; }

    /**
     * Returns the maximum position in local voxel coordinates.
     */
    glm::ivec3 getMaxPos() const { return minPos + extent; }

    /**
     * Returns the extent in world coordinates.
     */
    glm::vec3 getExtentWorld() const { return glm::vec3(extent) * voxelSize; }

    glm::vec3 getCenterPosWorld() const { return getMinPosWorld() + getExtentWorld() * 0.5f; }

    VoxelRegion toPrevLevelRegion() const
    {
        return VoxelRegion(minPos * 2, extent * 2, voxelSize / 2.0f);
    }

    VoxelRegion toNextLevelRegion() const
    {
        // extent + 1 is used to make sure that the upper bound is computed
        return VoxelRegion(minPos / 2, (extent + 1) / 2, voxelSize * 2.0f);
    }

    glm::ivec3 minPos;       // The minimum position in local voxel coordinates
    glm::ivec3 extent{ 0 };  // The extent of the region in local voxel coordinates 
    float voxelSize{ 0.0f }; // Voxel size in world coordinates

    // Note: the voxel size in local voxel coordinates is always 1
};
