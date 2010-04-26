#ifndef _dn_cudabvh_hpp_
#define _dn_cudabvh_hpp_

#include "dndefs.hpp"
#include "matrix4x4.hpp"
#include "cuda.hpp"
#include "bvhrt.hpp"

namespace dn
{
    class CudaBVH
    {
    public:
        CudaBVH(BVHRT* bvh);
        ~CudaBVH();

        CudaMemory* get_cuda_nodes() { return &cuda_nodes; }
        CudaMemory* get_cuda_aabbs_x() { return &cuda_aabbs_x; }
        CudaMemory* get_cuda_aabbs_y() { return &cuda_aabbs_y; }
        CudaMemory* get_cuda_aabbs_z() { return &cuda_aabbs_z; }
        CudaMemory* get_cuda_vertices() { return &cuda_vertices; }
        CudaMemory* get_cuda_woop_tris() { return &cuda_woop_tris; }

    private:
        void update();

        int convert(BVHRT::Node* node, int idx);

        struct CudaNode
        {
            int left_idx;
            int right_idx;
        };

        struct Vec4x3
        {
            Vector4f v[3];
        };

    private:
        BVHRT*       bvh;

        std::vector<CudaNode> nodes;
        std::vector<Vector4f> aabbs_x;
        std::vector<Vector4f> aabbs_y;
        std::vector<Vector4f> aabbs_z;
        std::vector<Vector4f> vertices;
        std::vector<Vec4x3> woop_tris;

        CudaMemory cuda_nodes;
        CudaMemory cuda_aabbs_x;
        CudaMemory cuda_aabbs_y;
        CudaMemory cuda_aabbs_z;
        CudaMemory cuda_vertices;
        CudaMemory cuda_woop_tris;
    };
}

#endif
