#ifndef _dn_cudavec_h_
#define _dn_cudavec_h_

__device__ float dot(float3 a, float3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

__device__ float dot(float4 a, float4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

__device__ float3 operator*(float3 a, float b)
{
    return make_float3(a.x * b, a.y * b, a.z * b);
}

__device__ float3 operator-(float3 a, float3 b)
{
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

__device__ float3 operator-(float3 a)
{
    return make_float3(-a.x, -a.y, -a.z);
}

__device__ float3 cross(float3 a, float3 b)
{
    return make_float3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x);
}

__device__ float length(float3 v)
{
    return sqrtf(dot(v, v));
}

__device__ float3 normalize(float3 v)
{
    return v * (1.f / length(v));
}

__device__ float3 xyz(float4 v)
{
    return make_float3(v.x, v.y, v.z);
}

#endif
