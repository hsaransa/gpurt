#include "cudavec.h"
#include <math_constants.h>

#ifndef BLOCK_WIDTH
#  define BLOCK_WIDTH 32
#  define BLOCK_HEIGHT 2
#endif

__constant__ int* result;
__constant__ int2*   nodes;
__constant__ float4* aabbs_x;
__constant__ float4* aabbs_y;
__constant__ float4* aabbs_z;
__constant__ float4* vertices;
__constant__ float4* woop_tris;
__constant__ int width, height;
__constant__ float4 matrix0;
__constant__ float4 matrix1;
__constant__ float4 matrix2;
__constant__ float4 matrix3;
__constant__ int2* zorder;

__device__ int warp_counter;

texture<int2, 1, cudaReadModeElementType> tex_nodes;
texture<float4, 1, cudaReadModeElementType> tex_aabbs_x;
texture<float4, 1, cudaReadModeElementType> tex_aabbs_y;
texture<float4, 1, cudaReadModeElementType> tex_aabbs_z;
texture<float4, 1, cudaReadModeElementType> tex_vertices;
texture<float4, 1, cudaReadModeElementType> tex_woop_tris;

struct shared_block
{
    float3 inv_dir;
};

__device__ float3 get_vertex(int i)
{
    float4 vv = vertices[i];
    //float4 vv = tex1Dfetch(tex_vertices, tri_i);//vertices[tri_i];
    return make_float3(vv.x, vv.y, vv.z);
}

extern "C" __global__ void bvh_trace()
{
    //int block = (blockIdx.z * gridDim.y + blockIdx.y) * gridDim.x + blockIdx.x;
    //int thread_idx = block * blockDim.x * blockDim.y + threadIdx.y * blockDim.x + threadIdx.x;

    // Init shared memory.

    __shared__ shared_block shared_memory[BLOCK_WIDTH * BLOCK_HEIGHT];
    shared_block* shared = &shared_memory[threadIdx.y * BLOCK_WIDTH + threadIdx.x];

    __shared__ volatile int ray_index[BLOCK_HEIGHT];
    __shared__ volatile int ray_count[BLOCK_HEIGHT];

    ray_index[threadIdx.y] = 0;
    ray_count[threadIdx.y] = 0;

    while (1)
    {
        // Fetch ray index.

#define QUEUE 1

        if (threadIdx.x == 0 && ray_count[threadIdx.y] == 0)
        {
            ray_index[threadIdx.y] = atomicAdd(&warp_counter, 32*QUEUE);
            ray_count[threadIdx.y] = 32*QUEUE;
        }

        int thread_idx = ray_index[threadIdx.y] + threadIdx.x;

        if (threadIdx.x == 0)
        {
            ray_index[threadIdx.y] += 32;
            ray_count[threadIdx.y] -= 32;
        }

        // Init pixel position.

        if (thread_idx >= width*height)
            break;

        int ix = zorder[thread_idx].x;
        int iy = zorder[thread_idx].y;

        int* resp = &result[iy * width + ix];
        *resp = 0x80FF80;

#if 1
        // Calculate view ray.

        float x0 = (ix + 0.5f) / (float)width * 2.f - 1.f;
        float y0 = (iy + 0.5f) / (float)height * 2.f - 1.f;

        float4 clip0 = make_float4(x0, y0, -1.f, 1.f);
        float4 hp0 = make_float4(dot(matrix0, clip0), dot(matrix1, clip0),
                dot(matrix2, clip0), dot(matrix3, clip0));
        float3 p0 = make_float3(hp0.x, hp0.y, hp0.z) * (1.0f / hp0.w);

        float4 clip1 = make_float4(x0, y0,  1.f, 1.f);
        float4 hp1 = make_float4(dot(matrix0, clip1), dot(matrix1, clip1),
                dot(matrix2, clip1), dot(matrix3, clip1));
        float3 p1 = make_float3(hp1.x, hp1.y, hp1.z) * (1.0f / hp1.w);

        float3 orig = p0;
        float3 dir = p1 - p0;

        float3 inv_dir;
        inv_dir.x = dir.x == 0.f ? 1e-32 : 1.f / dir.x;
        inv_dir.y = dir.y == 0.f ? 1e-32 : 1.f / dir.y;
        inv_dir.z = dir.z == 0.f ? 1e-32 : 1.f / dir.z;

        shared->inv_dir.x = inv_dir.x;
        shared->inv_dir.y = inv_dir.y;
        shared->inv_dir.z = inv_dir.z;

        // Trace.

        int stack[64];
        int sp = 0;
        int node_idx = 0;

        stack[63] = (int)resp; // spill result pointer to local stack

        float hit_t = CUDART_INF_F;

#define EXIT_NODE 0x66666666

        int debug = 0;
        while (node_idx != EXIT_NODE)
        {
            if (debug++ > 500)
                break;

            if (node_idx >= 0)
            {
                //float3 orig = make_float3(shared[0], shared[1], shared[2]);
                float3 orig_inv_dir = make_float3(-orig.x * shared->inv_dir.x, -orig.y * shared->inv_dir.y, -orig.z * shared->inv_dir.z);
                float tmin0, tmax0, tmin1, tmax1;

                tmin0 = tmin1 = 0.f;
                tmax0 = tmax1 = hit_t;

                {
                    float4 aabb = tex1Dfetch(tex_aabbs_x, node_idx);//aabbs_x[node_idx];
                    //float4 aabb = aabbs_x[node_idx];

                    float a0 = aabb.x * shared->inv_dir.x + orig_inv_dir.x;
                    float a1 = aabb.y * shared->inv_dir.x + orig_inv_dir.x;
                    tmin0 = fmaxf(fminf(a0, a1), tmin0);
                    tmax0 = fminf(fmaxf(a0, a1), tmax0);

                    float b0 = aabb.z * shared->inv_dir.x + orig_inv_dir.x;
                    float b1 = aabb.w * shared->inv_dir.x + orig_inv_dir.x;
                    tmin1 = fmaxf(fminf(b0, b1), tmin1);
                    tmax1 = fminf(fmaxf(b0, b1), tmax1);
                }

                {
                    float4 aabb = tex1Dfetch(tex_aabbs_y, node_idx);//aabbs_x[node_idx];
                    //float4 aabb = aabbs_y[node_idx];

                    float a0 = aabb.x * shared->inv_dir.y + orig_inv_dir.y;
                    float a1 = aabb.y * shared->inv_dir.y + orig_inv_dir.y;
                    tmin0 = fmaxf(tmin0, fminf(a0, a1));
                    tmax0 = fminf(tmax0, fmaxf(a0, a1));

                    float b0 = aabb.z * shared->inv_dir.y + orig_inv_dir.y;
                    float b1 = aabb.w * shared->inv_dir.y + orig_inv_dir.y;
                    tmin1 = fmaxf(tmin1, fminf(b0, b1));
                    tmax1 = fminf(tmax1, fmaxf(b0, b1));
                }

                {
                    float4 aabb = tex1Dfetch(tex_aabbs_z, node_idx);//aabbs_x[node_idx];
                    //float4 aabb = aabbs_z[node_idx];

                    float a0 = aabb.x * shared->inv_dir.z + orig_inv_dir.z;
                    float a1 = aabb.y * shared->inv_dir.z + orig_inv_dir.z;
                    tmin0 = fmaxf(tmin0, fminf(a0, a1));
                    tmax0 = fminf(tmax0, fmaxf(a0, a1));

                    float b0 = aabb.z * shared->inv_dir.z + orig_inv_dir.z;
                    float b1 = aabb.w * shared->inv_dir.z + orig_inv_dir.z;
                    tmin1 = fmaxf(tmin1, fminf(b0, b1));
                    tmax1 = fminf(tmax1, fmaxf(b0, b1));
                }

                int2 n = tex1Dfetch(tex_nodes, node_idx);

                if (tmin0 <= tmax0)
                {
                    if (tmin1 <= tmax1)
                    {
                        if (tmin1 < tmin0)
                        {
                            int t = n.x; n.x = n.y; n.y = t;
                        }
                        stack[sp++] = n.y;
                        node_idx = n.x;
                    }
                    else
                        node_idx = n.x;
                }
                else
                {
                    if (tmin1 <= tmax1)
                    {
                        node_idx = n.y;
                    }
                    else
                    {
                        if (sp)
                            node_idx = stack[--sp];
                        else
                            node_idx = EXIT_NODE;
                    }
                }
            }

            if (node_idx < 0)
            {
                int2 plop = tex1Dfetch(tex_nodes, -node_idx);
                int tri_i = plop.x;
                int tri_end = tri_i + plop.y;

                //float orig = make_float3(shared[0], shared[1], shared[2]) - v0;
                while (tri_i < tri_end)
                {

                    // Woop's triangle intersection wasn't as good.

#if 0
                    float3 dir = make_float3(
                            1.f / shared->inv_dir.x,
                            1.f / shared->inv_dir.y,
                            1.f / shared->inv_dir.z);

                    float4 v0 = woop_tris[tri_i];

                    float Oz = v0.w - dot(orig, xyz(v0));
                    float invDz = 1.f / dot(dir, xyz(v0));
                    float t = Oz * invDz;

                    if (t > 0.f && t < hit_t)
                    {
                        float4 v1 = woop_tris[tri_i+1];
                        float Ox = v1.w + dot(orig, xyz(v1));
                        float Dx = dot(dir, xyz(v1));
                        float u = Ox + t*Dx;
                        if (u >= 0.f)
                        {
                            float4 v2 = woop_tris[tri_i+2];
                            float Oy = v2.w + dot(orig, xyz(v2));
                            float Dy = dot(dir, xyz(v2));
                            float v = Oy + t*Dy;
                            if (v >= 0.f && u + v <= 1.f)
                            {
                                hit_t = t;
                                //hit_u = u;
                                //hit_v = v;
                                int r = t * 1000.f;
                                int g = t * 1500.f;
                                int b = t * 200.f;
                                *(int*)stack[63] = r | g << 8 | b << 16;
                            }
                        }
                    }

#else

                    // Moller-Trumbore triangle intersection

                    float3 v0 = get_vertex(tri_i);
                    float3 v1 = get_vertex(tri_i+1);
                    float3 v2 = get_vertex(tri_i+2);

                    float3 E1 = v1 - v0;
                    float3 E2 = v2 - v0;
                    float3 T = orig - v0;
                    float3 P = cross(dir, E2);

                    float inv_det = 1.f / dot(E1, P);

                    //float3 T = orig - v0;

                    float u = dot(T, P) * inv_det;

                    if (u >= 0.0f && u <= 1.0f)
                    {
                        float3 Q = cross(T, E1);

                        float v = dot(dir, Q) * inv_det;
                        if (v >= 0.0f && u + v <= 1.0f)
                        {
                            float t = dot(E2, Q) * inv_det;
                            if (t >= 0.0f && t < hit_t)
                            {
                                hit_t = t;
                                //hit_u = u;
                                //hit_v = v;

                                // "shading" is done for each intersection to
                                // save registers

                                // Refetch to save registers.
                                v0 = get_vertex(tri_i);
                                v1 = get_vertex(tri_i+1);
                                v2 = get_vertex(tri_i+2);

                                float3 n = cross(v1 - v0, v2 - v0);
                                n = normalize(n);

                                int r = (n.x * 0.5f + 0.5f) * 255.f;
                                int g = (n.y * 0.5f + 0.5f) * 255.f;
                                int b = (n.z * 0.5f + 0.5f) * 255.f;

                                *(int*)stack[63] = r | g << 8 | b << 16;
                            }
                        }
                    }
#endif

                    tri_i += 3;
                }

                if (sp)
                    node_idx = stack[--sp];
                else
                    node_idx = EXIT_NODE;
            }
        }
#endif
    }
}
