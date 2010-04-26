#ifndef _dn_vector4_hpp_
#define _dn_vector4_hpp_

#include "dndefs.hpp"
#include "vector3.hpp"

namespace dn
{
    template<typename T>
    class Vector4
    {
    public:
        Vector4() : x(0), y(0), z(0), w(0)
        {
        }

        Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w)
        {
        }

        Vector4(const Vector3<T>& v, T w) : x(v.x), y(v.y), z(v.z), w(w)
        {
        }

        Vector4(const Vector2<T>& v, T z, T w) : x(v.x), y(v.y), z(z), w(w)
        {
        }

        Vector3<T> xyz() const
        {
            return Vector3<T>(x, y, z);
        }

        Vector2<T> xy() const
        {
            return Vector2<T>(x, y);
        }

        T length() const
        {
            return std::sqrt(x*x + y*y + z*z + w*w);
        }

        Vector3<T> project() const
        {
            return Vector3<T>(x / w, y / w, z / w);
        }

        T& operator[](int i)
        {
            assert(i >= 0 && i < 4);
            assert(sizeof(T) == 4 || sizeof(T) == 8);
            // TODO: potentially dangerous, alignment and stuff...
            return (&x)[i];
        }

        const T& operator[](int i) const
        {
            assert(i >= 0 && i < 4);
            assert(sizeof(T) == 4 || sizeof(T) == 8);
            // TODO: potentially dangerous, alignment and stuff...
            return (&x)[i];
        }

    public:
        T x, y, z, w;
    };

    template<typename T>
    inline T length(const Vector4<T>& v)
    {
        return v.length();
    }

    template<typename T>
    inline T dot(const Vector4<T>& a, const Vector4<T>& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    template<typename T>
    inline Vector4<T> operator+(const Vector4<T>& a, const Vector4<T>& b)
    {
        return Vector4<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }

    template<typename T>
    inline Vector4<T>& operator+=(Vector4<T>& a, const Vector4<T>& b)
    {
        a.x += b.x;
        a.y += b.y;
        a.z += b.z;
        a.w += b.w;
        return a;
    }

    template<typename T>
    inline Vector4<T>& operator/=(Vector4<T>& a, T d)
    {
        a.x /= d;
        a.y /= d;
        a.z /= d;
        a.w /= d;
        return a;
    }

    typedef Vector4<float> Vector4f;
    typedef Vector4<double> Vector4d;
}

#endif
