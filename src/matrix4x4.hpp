#ifndef _dn_matrix4x4_hpp_
#define _dn_matrix4x4_hpp_

#include "dndefs.hpp"
#include "vector4.hpp"

namespace dn
{
    /*  0  1  2  3
     *  4  5  6  7
     *  8  9 10 11
     * 12 13 14 15
     */
    template<typename T>
    class Matrix4x4
    {
    public:
        Matrix4x4()
        {
            identity();
        }

        explicit Matrix4x4(const T* a)
        {
            for (int i = 0; i < 16; i++)
                m[i] = a[i];
        }

        bool is_identity()
        {
            return m[0]  == T(1) && m[1]  == T(0) && m[2]  == T(0) && m[3]  == T(0) &&
                   m[4]  == T(0) && m[5]  == T(1) && m[6]  == T(0) && m[7]  == T(0) &&
                   m[8]  == T(0) && m[9]  == T(0) && m[10] == T(1) && m[11] == T(0) &&
                   m[12] == T(0) && m[13] == T(0) && m[14] == T(0) && m[15] == T(1);
        }

        void identity()
        {
            for (int i = 0; i < 16; i++)
                m[i] = T(0);
            m[0] = m[5] = m[10] = m[15] = T(1);
        }

        void transpose()
        {
            std::swap(m[1], m[4]);
            std::swap(m[2], m[8]);
            std::swap(m[3], m[12]);
            std::swap(m[6], m[9]);
            std::swap(m[7], m[13]);
            std::swap(m[11], m[14]);
        }

        void scale(T s)
        {
            for (int i = 0; i < 16; i++)
                m[i] *= s;
        }

        T determinant()
        {
            return
                get(0, 3) * get(1, 2) * get(2, 1) * get(3, 0) -
                get(0, 2) * get(1, 3) * get(2, 1) * get(3, 0) -
                get(0, 3) * get(1, 1) * get(2, 2) * get(3, 0) +
                get(0, 1) * get(1, 3) * get(2, 2) * get(3, 0) +
                get(0, 2) * get(1, 1) * get(2, 3) * get(3, 0) -
                get(0, 1) * get(1, 2) * get(2, 3) * get(3, 0) -
                get(0, 3) * get(1, 2) * get(2, 0) * get(3, 1) +
                get(0, 2) * get(1, 3) * get(2, 0) * get(3, 1) +
                get(0, 3) * get(1, 0) * get(2, 2) * get(3, 1) -
                get(0, 0) * get(1, 3) * get(2, 2) * get(3, 1) -
                get(0, 2) * get(1, 0) * get(2, 3) * get(3, 1) +
                get(0, 0) * get(1, 2) * get(2, 3) * get(3, 1) +
                get(0, 3) * get(1, 1) * get(2, 0) * get(3, 2) -
                get(0, 1) * get(1, 3) * get(2, 0) * get(3, 2) -
                get(0, 3) * get(1, 0) * get(2, 1) * get(3, 2) +
                get(0, 0) * get(1, 3) * get(2, 1) * get(3, 2) +
                get(0, 1) * get(1, 0) * get(2, 3) * get(3, 2) -
                get(0, 0) * get(1, 1) * get(2, 3) * get(3, 2) -
                get(0, 2) * get(1, 1) * get(2, 0) * get(3, 3) +
                get(0, 1) * get(1, 2) * get(2, 0) * get(3, 3) +
                get(0, 2) * get(1, 0) * get(2, 1) * get(3, 3) -
                get(0, 0) * get(1, 2) * get(2, 1) * get(3, 3) -
                get(0, 1) * get(1, 0) * get(2, 2) * get(3, 3) +
                get(0, 0) * get(1, 1) * get(2, 2) * get(3, 3);
        }

        bool invert()
        {
            T det = determinant();
            if (det == 0)
                return false;
            T r[16];
            r[0*4+0] = get(1, 2)*get(2, 3)*get(3, 1) -
                get(1, 3)*get(2, 2)*get(3, 1) +
                get(1, 3)*get(2, 1)*get(3, 2) -
                get(1, 1)*get(2, 3)*get(3, 2) -
                get(1, 2)*get(2, 1)*get(3, 3) +
                get(1, 1)*get(2, 2)*get(3, 3);
            r[0*4+1] = get(0, 3)*get(2, 2)*get(3, 1) -
                get(0, 2)*get(2, 3)*get(3, 1) -
                get(0, 3)*get(2, 1)*get(3, 2) +
                get(0, 1)*get(2, 3)*get(3, 2) +
                get(0, 2)*get(2, 1)*get(3, 3) -
                get(0, 1)*get(2, 2)*get(3, 3);
            r[0*4+2] = get(0, 2)*get(1, 3)*get(3, 1) -
                get(0, 3)*get(1, 2)*get(3, 1) +
                get(0, 3)*get(1, 1)*get(3, 2) -
                get(0, 1)*get(1, 3)*get(3, 2) -
                get(0, 2)*get(1, 1)*get(3, 3) +
                get(0, 1)*get(1, 2)*get(3, 3);
            r[0*4+3] = get(0, 3)*get(1, 2)*get(2, 1) -
                get(0, 2)*get(1, 3)*get(2, 1) -
                get(0, 3)*get(1, 1)*get(2, 2) +
                get(0, 1)*get(1, 3)*get(2, 2) +
                get(0, 2)*get(1, 1)*get(2, 3) -
                get(0, 1)*get(1, 2)*get(2, 3);
            r[1*4+0] = get(1, 3)*get(2, 2)*get(3, 0) -
                get(1, 2)*get(2, 3)*get(3, 0) -
                get(1, 3)*get(2, 0)*get(3, 2) +
                get(1, 0)*get(2, 3)*get(3, 2) +
                get(1, 2)*get(2, 0)*get(3, 3) -
                get(1, 0)*get(2, 2)*get(3, 3);
            r[1*4+1] = get(0, 2)*get(2, 3)*get(3, 0) -
                get(0, 3)*get(2, 2)*get(3, 0) +
                get(0, 3)*get(2, 0)*get(3, 2) -
                get(0, 0)*get(2, 3)*get(3, 2) -
                get(0, 2)*get(2, 0)*get(3, 3) +
                get(0, 0)*get(2, 2)*get(3, 3);
            r[1*4+2] = get(0, 3)*get(1, 2)*get(3, 0) -
                get(0, 2)*get(1, 3)*get(3, 0) -
                get(0, 3)*get(1, 0)*get(3, 2) +
                get(0, 0)*get(1, 3)*get(3, 2) +
                get(0, 2)*get(1, 0)*get(3, 3) -
                get(0, 0)*get(1, 2)*get(3, 3);
            r[1*4+3] = get(0, 2)*get(1, 3)*get(2, 0) -
                get(0, 3)*get(1, 2)*get(2, 0) +
                get(0, 3)*get(1, 0)*get(2, 2) -
                get(0, 0)*get(1, 3)*get(2, 2) -
                get(0, 2)*get(1, 0)*get(2, 3) +
                get(0, 0)*get(1, 2)*get(2, 3);
            r[2*4+0] = get(1, 1)*get(2, 3)*get(3, 0) -
                get(1, 3)*get(2, 1)*get(3, 0) +
                get(1, 3)*get(2, 0)*get(3, 1) -
                get(1, 0)*get(2, 3)*get(3, 1) -
                get(1, 1)*get(2, 0)*get(3, 3) +
                get(1, 0)*get(2, 1)*get(3, 3);
            r[2*4+1] = get(0, 3)*get(2, 1)*get(3, 0) -
                get(0, 1)*get(2, 3)*get(3, 0) -
                get(0, 3)*get(2, 0)*get(3, 1) +
                get(0, 0)*get(2, 3)*get(3, 1) +
                get(0, 1)*get(2, 0)*get(3, 3) -
                get(0, 0)*get(2, 1)*get(3, 3);
            r[2*4+2] = get(0, 1)*get(1, 3)*get(3, 0) -
                get(0, 3)*get(1, 1)*get(3, 0) +
                get(0, 3)*get(1, 0)*get(3, 1) -
                get(0, 0)*get(1, 3)*get(3, 1) -
                get(0, 1)*get(1, 0)*get(3, 3) +
                get(0, 0)*get(1, 1)*get(3, 3);
            r[2*4+3] = get(0, 3)*get(1, 1)*get(2, 0) -
                get(0, 1)*get(1, 3)*get(2, 0) -
                get(0, 3)*get(1, 0)*get(2, 1) +
                get(0, 0)*get(1, 3)*get(2, 1) +
                get(0, 1)*get(1, 0)*get(2, 3) -
                get(0, 0)*get(1, 1)*get(2, 3);
            r[3*4+0] = get(1, 2)*get(2, 1)*get(3, 0) -
                get(1, 1)*get(2, 2)*get(3, 0) -
                get(1, 2)*get(2, 0)*get(3, 1) +
                get(1, 0)*get(2, 2)*get(3, 1) +
                get(1, 1)*get(2, 0)*get(3, 2) -
                get(1, 0)*get(2, 1)*get(3, 2);
            r[3*4+1] = get(0, 1)*get(2, 2)*get(3, 0) -
                get(0, 2)*get(2, 1)*get(3, 0) +
                get(0, 2)*get(2, 0)*get(3, 1) -
                get(0, 0)*get(2, 2)*get(3, 1) -
                get(0, 1)*get(2, 0)*get(3, 2) +
                get(0, 0)*get(2, 1)*get(3, 2);
            r[3*4+2] = get(0, 2)*get(1, 1)*get(3, 0) -
                get(0, 1)*get(1, 2)*get(3, 0) -
                get(0, 2)*get(1, 0)*get(3, 1) +
                get(0, 0)*get(1, 2)*get(3, 1) +
                get(0, 1)*get(1, 0)*get(3, 2) -
                get(0, 0)*get(1, 1)*get(3, 2);
            r[3*4+3] = get(0, 1)*get(1, 2)*get(2, 0) -
                get(0, 2)*get(1, 1)*get(2, 0) +
                get(0, 2)*get(1, 0)*get(2, 1) -
                get(0, 0)*get(1, 2)*get(2, 1) -
                get(0, 1)*get(1, 0)*get(2, 2) +
                get(0, 0)*get(1, 1)*get(2, 2);
            *this = Matrix4x4<T>(r);
            scale(T(1) / det);
            return true;
        }

        inline Vector4<T> row(int i) const
        {
            assert(i >= 0 && i < 4);
            return Vector4<T>(m[i*4+0], m[i*4+1], m[i*4+2], m[i*4+3]);
        }

        inline void set_row(int i, const Vector4<T>& v)
        {
            assert(i >= 0 && i < 4);
            m[i*4+0] = v.x;
            m[i*4+1] = v.y;
            m[i*4+2] = v.z;
            m[i*4+3] = v.w;
        }

        inline Vector4<T> column(int i) const
        {
            assert(i >= 0 && i < 4);
            return Vector4<T>(m[0*4+i], m[1*4+i], m[2*4+i], m[3*4+i]);
        }

        inline void set_column(int i, const Vector4<T>& v)
        {
            assert(i >= 0 && i < 4);
            m[0*4+i] = v.x;
            m[1*4+i] = v.y;
            m[2*4+i] = v.z;
            m[3*4+i] = v.w;
        }

        inline T get(int i, int j) const
        {
            assert(i >= 0 && i < 4 && j >= 0 && j < 4);
            return m[i*4+j];
        }

        const T* data() const
        {
            return m;
        }

        T* data()
        {
            return m;
        }

        const T& operator[](int i) const
        {
            assert(i >= 0 && i < 16);
            return m[i];
        }

        T& operator[](int i)
        {
            assert(i >= 0 && i < 16);
            return m[i];
        }

    private:
        T m[16];
    };

    template<typename T>
    Vector4<T> operator*(const Matrix4x4<T>& m, const Vector4<T>& v)
    {
        return Vector4<T>(
            m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w,
            m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w,
            m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w,
            m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w);
    }

    template<typename T>
    Matrix4x4<T> operator*(const Matrix4x4<T>& a, const Matrix4x4<T>& b)
    {
        Matrix4x4<T> r;

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                r[i*4+j] = dot(a.row(i), b.column(j));
            }
        }

        return r;
    }

    template<typename T>
    Matrix4x4<T> transpose(const Matrix4x4<T>& m)
    {
        Matrix4x4<T> m2 = m;
        m2.transpose();
        return m2;
    }

    template<typename T>
    Matrix4x4<T> invert(const Matrix4x4<T>& m)
    {
        Matrix4x4<T> m2 = m;
        if (!m2.invert())
        {
            throw std::runtime_error("matrix is not invertible");
        }
        return m2;
    }

    template<typename T>
    Matrix4x4<T> rotate(const Vector3<T>& vec, T rad)
    {
        // This formula is from OpenGL manpages.

        Vector3<T> v = normalize(vec);
        T x = v.x;
        T y = v.y;
        T z = v.z;
        T c = std::cos(rad);
        T s = std::sin(rad);

        Matrix4x4<T> m;

        m[0]  = x * x * (T(1) - c) + c;
        m[1]  = x * y * (T(1) - c) - z * s;
        m[2]  = x * z * (T(1) - c) + y * s;

        m[4]  = y * x * (T(1) - c) + z * s;
        m[5]  = y * y * (T(1) - c) + c;
        m[6]  = y * z * (T(1) - c) - x * s;

        m[8]  = z * x * (T(1) - c) - y * s;
        m[9]  = z * y * (T(1) - c) + x * s;
        m[10] = z * z * (T(1) - c) + c;

        //m.transpose();

        return m;
    }

    template<typename T>
    Matrix4x4<T> translate(const Vector3<T>& vec)
    {
        Matrix4x4<T> m;

        m[3] = vec.x;
        m[7] = vec.y;
        m[11] = vec.z;

        //m.transpose();

        return m;
    }

    template<typename T>
    Matrix4x4<T> scale(const Vector3<T>& vec)
    {
        Matrix4x4<T> m;

        m[0] = vec.x;
        m[5] = vec.y;
        m[10] = vec.z;

        return m;
    }

    template<typename T>
    Matrix4x4<T> perspective(T fovy, T aspect, T zNear, T zFar)
    {
        assert(zNear > T(0) && zNear < zFar);

        Matrix4x4<T> m;
        m.identity();

        T f = T(1) / std::tan(fovy / T(2));

        m[0*4+0] = f / aspect;
        m[1*4+1] = f;
        m[2*4+2] = (zFar + zNear) / (zNear - zFar);
        m[2*4+3] = T(2) * zFar * zNear / (zNear - zFar);
        m[3*4+2] = T(-1);
        m[3*4+3] = T(0);

        return m;
    }

    template<typename T>
    Matrix4x4<T> ortho(const Vector3<T>& p0, const Vector3<T>& p1)
    {
        T left = p0.x;
        T right = p1.x;
        T bottom = p0.y;
        T top = p1.y;
        T nearval = p0.z;
        T farval = p1.z;

        Matrix4x4<T> m;
        m[0] = T(2) / (right - left);
        m[5] = T(2) / (top - bottom);
        m[10] = T(2) / (farval - nearval);
        m[3] = -(right + left) / (right - left);
        m[7] = -(top + bottom) / (top - bottom);
        m[11] = -(farval + nearval) / (farval - nearval);

        return m;
    }

    template<typename T>
    Matrix4x4<T> ortho2d(const Vector2<T>& p0, const Vector2<T>& p1)
    {
        return ortho<T>(Vector3<T>(p0, T(-1)), Vector3<T>(p1, T(1)));
    }

    template<typename T>
    Matrix4x4<T> look_at(const Vector3<T>& eye, const Vector3<T>& center, const Vector3<T>& up)
    {
        Vector3<T> f = normalize(center - eye);
        Vector3<T> s = normalize(cross(f, up));
        Vector3<T> u = cross(s, f);

        Matrix4x4<T> m;
        m[0] = s.x;
        m[1] = s.y;
        m[2] = s.z;
        m[4] = u.x;
        m[5] = u.y;
        m[6] = u.z;
        m[8] = -f.x;
        m[9] = -f.y;
        m[10] = -f.z;

        return m * translate(-eye);
    }

    template<typename T, typename F>
    Matrix4x4<T> convert_to(const Matrix4x4<F>& m)
    {
        Matrix4x4<T> r;

        for (int i = 0; i < 16; i++)
            r[i] = T(m[i]);

        return r;
    }

    typedef Matrix4x4<float> Matrix4x4f;
    typedef Matrix4x4<double> Matrix4x4d;
    typedef Matrix4x4<float> Matrix4f;
    typedef Matrix4x4<double> Matrix4d;
}

#endif
