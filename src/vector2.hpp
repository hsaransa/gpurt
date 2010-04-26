#ifndef _dn_vector2_hpp_
#define _dn_vector2_hpp_

#include "dndefs.hpp"
#include <cmath>

namespace dn
{
    template<typename T>
    class Vector2
    {
    public:
        Vector2() : x(0), y(0)
        {
        }

        Vector2(T x, T y) : x(x), y(y)
        {
        }

        explicit Vector2(const T* v) : x(v[0]), y(v[1])
        {
        }

        T length() const
        {
            return std::sqrt(x*x + y*y);
        }

        T& operator[](int i)
        {
            assert(i >= 0 && i < 2);
            assert(sizeof(T) == 4 || sizeof(T) == 8);
            // TODO: potentially dangerous, alignment and stuff...
            return (&x)[i];
        }

        const T& operator[](int i) const
        {
            assert(i >= 0 && i < 2);
            assert(sizeof(T) == 4 || sizeof(T) == 8);
            // TODO: potentially dangerous, alignment and stuff...
            return (&x)[i];
        }

    public:
        T x, y;
    };

    template<typename T>
    inline bool operator==(const Vector2<T>& a, const Vector2<T>& b)
    {
        return a.x == b.x && a.y == b.y;
    }

    template<typename T>
    inline Vector2<T> operator*(const Vector2<T>& a, T s)
    {
        return Vector2<T>(a.x * s, a.y * s);
    }

    template<typename T>
    inline Vector2<T> operator/(const Vector2<T>& a, T s)
    {
        return Vector2<T>(a.x / s, a.y / s);
    }

    template<typename T>
    inline Vector2<T> operator+(const Vector2<T>& a, const Vector2<T>& b)
    {
        return Vector2<T>(a.x + b.x, a.y + b.y);
    }

    template<typename T>
    inline Vector2<T> operator-(const Vector2<T>& a, const Vector2<T>& b)
    {
        return Vector2<T>(a.x - b.x, a.y - b.y);
    }

    template<typename T>
        inline Vector2<T> operator-(const Vector2<T>& b)
        {
            return Vector2<T>(-b.x, -b.y);
        }

    template<typename T>
        inline T dot(const Vector2<T>& a, const Vector2<T>& b)
        {
            return a.x * b.x + a.y * b.y;
        }

    template<typename T>
        inline T length(const Vector2<T>& a)
        {
            return a.length();
        }

    // Rotates a 90 degrees in counter-clockwise order.
    // Don't change this, use negation instead.
    template<typename T>
        inline Vector2<T> perpendicular(const Vector2<T>& a)
        {
            return Vector2<T>(-a.y, a.x);
        }

    template<typename T>
        inline Vector2<T> normalize(const Vector2<T>& a)
        {
            T len = length(a);
            // TODO: throw exception when len == 0
            return a / len;
        }

    template<typename T>
    inline T cross(const Vector2<T>& a, const Vector2<T>& b)
    {
        return a.x * b.y - a.y * b.x;
    }

    template<typename T>
    inline bool circle_through_three_points(const Vector2<T> &a, const Vector2<T> &b, const Vector2<T> &c, Vector2<T> &p, float &r)
    {
        T tmp = a.y * (b.x - c.x) + b.y * c.x - b.x * c.y + a.x * (-b.y + c.y);

        if (tmp == 0.0f || tmp * tmp == 0.0f)
            return false;

        T rr = std::sqrt(
                (((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)) *
                 ((a.x - c.x) * (a.x - c.x) + (a.y - c.y) * (a.y - c.y)) *
                 ((b.x - c.x) * (b.x - c.x) + (b.y - c.y) * (b.y - c.y))
                 / (tmp * tmp))) / 2.0f;

        tmp *= 2.0f;

        T px =
            (b.y * c.x * c.x +
             -(b.x * b.x + b.y * b.y) * c.y +
             b.y * c.y * c.y +
             a.x * a.x * (-b.y + c.y) +
             a.y * a.y * (-b.y + c.y) +
             a.y * (b.x * b.x + b.y * b.y - c.x * c.x - c.y * c.y)) / tmp;

        T py =
            (a.x * a.x * (b.x - c.x) +
             a.y * a.y * (b.x - c.x) +
             c.x * (b.x * b.x + b.y * b.y - b.x * c.x) +
             -b.x * c.y * c.y +
             a.x * (-b.x * b.x - b.y * b.y + c.x * c.x + c.y * c.y)) / tmp;

        r = rr;
        p.x = px;
        p.y = py;

        return true;
    }

    typedef Vector2<float> Vector2f;
    typedef Vector2<double> Vector2d;
    typedef Vector2<int> Vector2i;
}

#endif
