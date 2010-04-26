#ifndef _dn_dndefs_hpp_
#define _dn_dndefs_hpp_

#include <cassert>

#define DN_ARRAY_LENGTH(ary) (sizeof(ary) / sizeof((ary)[0]))

namespace dn
{
    class HostMemory;
    class DiskMemory;
    class CudaMemory;
    class CudaTexture;
    class Material;

    struct Primitive;

    template<typename T> class AABB;
    typedef AABB<float> AABBf;
    typedef AABB<double> AABBd;

    template<typename T> class Vector4;
    typedef Vector4<float> Vector4f;
    typedef Vector4<double> Vector4d;
}

#endif
