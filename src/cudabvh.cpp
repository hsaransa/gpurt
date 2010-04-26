#include "cudabvh.hpp"
#include "bvhrt.hpp"
#include <stdio.h>

using namespace dn;

CudaBVH::CudaBVH(BVHRT* bvh)
:   bvh(bvh)
{
    update();
}

CudaBVH::~CudaBVH()
{
}

struct CudaNode
{
    int left_idx;
    int right_idx;
};

void CudaBVH::update()
{
    BVHRT::Node* root = bvh->get_root();
    int count = root->count();

    nodes.resize(count);
    aabbs_x.resize(count);
    aabbs_y.resize(count);
    aabbs_z.resize(count);
    vertices.clear();
    woop_tris.clear();

    int ret = convert(root, 0);
    assert(ret == count);

    this->cuda_nodes.fill(nodes);
    this->cuda_aabbs_x.fill(aabbs_x);
    this->cuda_aabbs_y.fill(aabbs_y);
    this->cuda_aabbs_z.fill(aabbs_z);
    this->cuda_vertices.fill(vertices);
    this->cuda_woop_tris.fill(woop_tris);
}

int CudaBVH::convert(BVHRT::Node* node, int idx)
{
    assert(node);
    assert(idx < (int)nodes.size());

    int ret = idx + 1;

    if (node->left)
    {
        // Negative index means leaf.
        nodes[idx].left_idx = (node->left->left) ? ret : -ret;
        ret = convert(node->left, ret);
        nodes[idx].right_idx = (node->right->left) ? ret : -ret;
        ret = convert(node->right, ret);

        aabbs_x[idx].x = node->left->aabb.min.x;
        aabbs_x[idx].y = node->left->aabb.max.x;
        aabbs_x[idx].z = node->right->aabb.min.x;
        aabbs_x[idx].w = node->right->aabb.max.x;

        aabbs_y[idx].x = node->left->aabb.min.y;
        aabbs_y[idx].y = node->left->aabb.max.y;
        aabbs_y[idx].z = node->right->aabb.min.y;
        aabbs_y[idx].w = node->right->aabb.max.y;

        aabbs_z[idx].x = node->left->aabb.min.z;
        aabbs_z[idx].y = node->left->aabb.max.z;
        aabbs_z[idx].z = node->right->aabb.min.z;
        aabbs_z[idx].w = node->right->aabb.max.z;
    }
    else
    {
        int n = (int)node->primitives.size();

        nodes[idx].left_idx = (int)vertices.size();
        nodes[idx].right_idx = (int)node->primitives.size() * 3;

        for (int i = 0; i < n; i++)
        {
            const Primitive& prim = bvh->get_primitive(node->primitives[i]);
            vertices.push_back(Vector4f(prim.v0, 1.f));
            vertices.push_back(Vector4f(prim.v1, 1.f));
            vertices.push_back(Vector4f(prim.v2, 1.f));

#if 0
            Matrix4x4f m;
            m.set_column(0, Vector4f(prim.v0 - prim.v2, 0.f));
            m.set_column(1, Vector4f(prim.v1 - prim.v2, 0.f));
            m.set_column(2, Vector4f(cross(prim.v0 - prim.v2, prim.v1 - prim.v2) - prim.v2, 0.f));
            m.set_column(3, Vector4f(prim.v2, 0.f));

            m = invert(m);

            Vec4x3 v;
            v.v[0] = Vector4f(m.get(2, 0), m.get(2, 1), m.get(2, 2), -m.get(2, 3));
            v.v[1] = Vector4f(m.get(0, 0), m.get(0, 1), m.get(0, 2),  m.get(0, 3));
            v.v[2] = Vector4f(m.get(1, 0), m.get(1, 1), m.get(1, 2),  m.get(1, 3));

            woop_tris.push_back(v);
#endif
        }
    }

    return ret;
}
