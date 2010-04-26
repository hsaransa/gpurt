#ifndef _dn_aabb_hpp_
#define _dn_aabb_hpp_

#include "vector3.hpp"
#include <boost/numeric/conversion/bounds.hpp>
#include <algorithm>

namespace dn
{
    template<typename T>
    class AABB
    {
    public:
        AABB()
        {
            min.x = boost::numeric::bounds<T>::highest();
            min.y = boost::numeric::bounds<T>::highest();
            min.z = boost::numeric::bounds<T>::highest();
            max.x = boost::numeric::bounds<T>::lowest();
            max.y = boost::numeric::bounds<T>::lowest();
            max.z = boost::numeric::bounds<T>::lowest();
        }

        AABB(const Vector3<T>& a, const Vector3<T>& b)
        :   min(a), max(b)
        {
        }

        void grow(const Vector3<T>& v)
        {
            min.x = std::min(min.x, v.x);
            min.y = std::min(min.y, v.y);
            min.z = std::min(min.z, v.z);
            max.x = std::max(max.x, v.x);
            max.y = std::max(max.y, v.y);
            max.z = std::max(max.z, v.z);
        }

        void grow(const AABB<T>& a)
        {
            grow(a.min);
            grow(a.max);
        }

        bool contains(const Vector3<T>& v) const
        {
            return
                v.x >= min.x && v.x <= max.x &&
                v.y >= min.y && v.y <= max.y &&
                v.z >= min.z && v.z <= max.z;
        }

        Vector3<T> get_diagonal() const
        {
            return max - min;
        }

        T get_volume() const
        {
            Vector3<T> diag = get_diagonal();
            return diag.x * diag.y * diag.z;
        }

        T get_surface_area() const
        {
            Vector3f diag = get_diagonal();
            return T(2) * (diag.x * diag.y + diag.x * diag.z + diag.y * diag.z);
        }

        AABB<T> sub_aabb(int x, int y, int z, int w, int h, int d) const
        {
            AABB<T> aabb;

            T fx1 = T(x) / T(w);
            T fy1 = T(y) / T(h);
            T fz1 = T(z) / T(d);
            T fx2 = T(x+1) / T(w);
            T fy2 = T(y+1) / T(h);
            T fz2 = T(z+1) / T(d);

            aabb.min.x = max.x * fx1 + min.x * (T(1) - fx1);
            aabb.min.y = max.y * fy1 + min.y * (T(1) - fy1);
            aabb.min.z = max.z * fz1 + min.z * (T(1) - fz1);
            aabb.max.x = max.x * fx2 + min.x * (T(1) - fx2);
            aabb.max.y = max.y * fy2 + min.y * (T(1) - fy2);
            aabb.max.z = max.z * fz2 + min.z * (T(1) - fz2);

            return aabb;
        }

        bool is_valid() const
        {
            return min.x <= max.x && min.y <= max.y && min.z <= max.z;
        }

    public:
        Vector3<T> min, max;
    };

    typedef AABB<float> AABBf;
    typedef AABB<double> AABBd;
}

#endif
