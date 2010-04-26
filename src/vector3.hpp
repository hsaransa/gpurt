#ifndef _dn_vector3_hpp_
#define _dn_vector3_hpp_

#include "dndefs.hpp"
#include "vector2.hpp"
#include <stdexcept>

namespace dn
{
    template<typename T>
    class Vector3
    {
    public:
        Vector3() : x(T(0)), y(T(0)), z(T(0))
        {
        }

        Vector3(T x, T y, T z) : x(x), y(y), z(z)
        {
        }

        Vector3(Vector2<T> v, T z) : x(v.x), y(v.y), z(z)
        {
        }

        explicit Vector3(const T* v) : x(v[0]), y(v[1]), z(v[2])
        {
        }

        T length() const
        {
            return std::sqrt(x*x + y*y + z*z);
        }

        T length_squared() const
        {
            return x*x + y*y + z*z;
        }

        Vector2<T> xy() const
        {
            return Vector2<T>(x, y);
        }

        bool is_zero() const
        {
            return x == T(0) && y == T(0) && z == T(0);
        }

        bool normalize()
        {
            T len = length();
            if (len == T(0))
                return false;
            x /= len;
            y /= len;
            z /= len;
            return true;
        }

        T& operator[](int i)
        {
            assert(i >= 0 && i < 3);
            assert(sizeof(T) == 4 || sizeof(T) == 8);
            // TODO: potentially dangerous, alignment and stuff...
            return (&x)[i];
        }

        const T& operator[](int i) const
        {
            assert(i >= 0 && i < 3);
            assert(sizeof(T) == 4 || sizeof(T) == 8);
            // TODO: potentially dangerous, alignment and stuff...
            return (&x)[i];
        }

    public:
        T x, y, z;
    };

    template<typename T>
    inline bool operator==(const Vector3<T>& a, const Vector3<T>& b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    template<typename T>
    inline Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Vector3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    template<typename T>
    inline Vector3<T>& operator+=(Vector3<T>& a, const Vector3<T>& b)
    {
        a.x += b.x;
        a.y += b.y;
        a.z += b.z;
        return a;
    }

    template<typename T>
    inline Vector3<T>& operator*=(Vector3<T>& a, T s)
    {
        a.x *= s;
        a.y *= s;
        a.z *= s;
        return a;
    }

    template<typename T>
    inline Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Vector3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    template<typename T>
    inline Vector3<T> operator-(const Vector3<T>& b)
    {
        return Vector3<T>(-b.x, -b.y, -b.z);
    }

    template<typename T>
    inline Vector3<T> scale(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Vector3<T>(a.x * b.x, a.y * b.y, a.z * b.z);
    }

    template<typename T>
    inline Vector3<T> operator*(const Vector3<T>& a, const Vector3<T>& b)
    {
        return scale(a, b);
    }

    template<typename T>
    inline Vector3<T> operator*(const Vector3<T>& a, T s)
    {
        return Vector3<T>(a.x * s, a.y * s, a.z * s);
    }

    template<typename T>
    inline T dot(const Vector3<T>& a, const Vector3<T>& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template<typename T>
    inline Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Vector3<T>(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x);
    }

    template<typename T>
    inline Vector3<T> normalize(const Vector3<T>& a)
    {
        Vector3<T> b = a;
        // Is this a bad idea?
        if (!b.normalize())
            throw std::runtime_error("bad vector to normalize");
        return b;
    }

    template<typename T>
    inline T length(const Vector3<T>& v)
    {
        return v.length();
    }

    template<typename T>
    inline Vector3<T> min_values(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Vector3<T>(
            a.x < b.x ? a.x : b.x,
            a.y < b.y ? a.y : b.y,
            a.z < b.z ? a.z : b.z);
    }

    template<typename T>
    inline Vector3<T> max_values(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Vector3<T>(
            a.x >= b.x ? a.x : b.x,
            a.y >= b.y ? a.y : b.y,
            a.z >= b.z ? a.z : b.z);
    }

    template<typename T, typename F>
    inline Vector3<T> convert_to(const Vector3<F>& v)
    {
        return Vector3<T>(T(v.x), T(v.y), T(v.z));
    }

    typedef Vector3<int> Vector3i;
    typedef Vector3<float> Vector3f;
    typedef Vector3<double> Vector3d;
}

#endif
