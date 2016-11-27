#pragma once
#include <glm/glm.hpp>
#include "BBox.h"

class BBox;

namespace intersection
{
    /**
    * @brief   BBox/Triangle overlap test. Algorithm from "Fast Parallel Surface and Solid Voxelization on GPUs" by Michael Schwarz and Hans-Peter Seidel.
    */

    bool bboxTriangle(const BBox& b, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);

    class TriangleBBox
    {
    public:
        void setTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
        void setBBoxScale(const glm::vec3& scale);
        bool test(const BBox& b);

        glm::vec3 v0, v1, v2;
        glm::vec3 n;
        BBox triangleBBox;
        glm::vec3 dp;

        glm::vec2 n_yz_e0;
        glm::vec2 n_yz_e1;
        glm::vec2 n_yz_e2;

        glm::vec2 n_zx_e0;
        glm::vec2 n_zx_e1;
        glm::vec2 n_zx_e2;

        glm::vec2 n_xy_e0;
        glm::vec2 n_xy_e1;
        glm::vec2 n_xy_e2;

        float d1;
        float d2;

        float d_xy_e0;
        float d_xy_e1;
        float d_xy_e2;

        float d_zx_e0;
        float d_zx_e1;
        float d_zx_e2;

        float d_yz_e0;
        float d_yz_e1;
        float d_yz_e2;
    };
}
