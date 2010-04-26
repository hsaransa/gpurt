#ifndef _dn_primitive_hpp_
#define _dn_primitive_hpp_

#include "dndefs.hpp"
#include "aabb.hpp"
#include "matrix4x4.hpp"

namespace dn
{
    struct Primitive
    {
        enum Type
        {
            TRIANGLE,
            PARALLELOGRAM,
            SPHERE,
            POINT,
            NONE
        };

        enum Attribute
        {
            NORMAL = 0,
        };

        Primitive() { set_type(NONE); set_attributes(0); }
        Primitive(Type t) { set_type(t); set_attributes(0); }
        Primitive(Type t, const Vector3f& v0, const Vector3f& v1, const Vector3f& v2)
        : v0(v0), v1(v1), v2(v2)
        {
            set_type(t);
            set_attributes(0);
        }

        void set_type(Type t)
        {
            type = (int)t;
        }

        Type get_type() const
        {
            return (Type)type;
        }

        void set_attributes(int m)
        {
            attributes = m;
        }

        void add_attribute(Attribute attr)
        {
            attributes |= 1 << attr;
        }

        bool has_attribute(Attribute attr) const
        {
            return !!(attributes & (1 << NORMAL));
        }

        Vector3f get_triangle_v0() const { return v0; }
        Vector3f get_triangle_v1() const { return v1; }
        Vector3f get_triangle_v2() const { return v2; }
        Vector3f get_point_position() const { return v0; }

        AABBf get_aabb() const
        {
            AABBf aabb;
            aabb.grow(v0);

            if (type == TRIANGLE)
            {
                aabb.grow(v1);
                aabb.grow(v2);
            }
            else if (type == PARALLELOGRAM)
            {
                aabb.grow(v0+v1);
                aabb.grow(v0+v2);
                aabb.grow(v0+v1+v2);
            }
            else if (type == POINT)
                aabb.grow(v0);

            return aabb;
        }

        bool intersect(const Vector3f& O, const Vector3f& D, float& _t, float& _u, float& _v) const
        {
            switch (type)
            {
            case TRIANGLE:
                {
                    const Vector3f E1 = v1 - v0;
                    const Vector3f E2 = v2 - v0;
                    const Vector3f P = cross(D, E2);

                    float det = dot(E1, P);
                    float inv_det = 1.0f / det;

                    // Epsilon test goes here.

                    const Vector3f T = O - v0;

                    float u = dot(T, P) * inv_det;

                    if (u < 0.0f || u > 1.0f)
                        return false;

                    const Vector3f Q = cross(T, E1);

                    float v = dot(D, Q) * inv_det;
                    if (v < 0.0f || u + v > 1.0f)
                        return false;

                    float t = dot(E2, Q) * inv_det;
                    if (t < 0.0f)
                        return false;

                    _t = t;
                    _u = u;
                    _v = v;

                    return true;
                }

            case PARALLELOGRAM:
                {
                    const Vector3f E1 = v1;
                    const Vector3f E2 = v2;
                    const Vector3f P = cross(D, E2);

                    float det = dot(E1, P);
                    float inv_det = 1.0f / det;

                    // Epsilon test goes here.

                    const Vector3f T = O - v0;

                    float u = dot(T, P) * inv_det;

                    if (u < 0.0f || u > 1.0f)
                        return false;

                    const Vector3f Q = cross(T, E1);

                    float v = dot(D, Q) * inv_det;
                    if (v < 0.0f || v > 1.0f)
                        return false;

                    float t = dot(E2, Q) * inv_det;
                    if (t < 0.0f)
                        return false;

                    _t = t;
                    _u = u;
                    _v = v;

                    return true;
                }
            }

            return false;
        }

        Vector3f get_normal(float u, float v) const
        {
            if (type == TRIANGLE)
                return cross(v1 - v0, v2 - v0);
            else if (type == PARALLELOGRAM)
                return cross(v1, v2);
            else
                return v2;
        }

        Vector3f sample_position(float u, float v) const
        {
            if (type == PARALLELOGRAM)
                return v0 + v1 * u + v2 * v;
            assert(!"todo");
        }

        void transform(const Matrix4x4f& m)
        {
            v0 = (m * Vector4f(v0, 1.0f)).xyz();
            v1 = (m * Vector4f(v1, 1.0f)).xyz();
            v2 = (m * Vector4f(v2, 1.0f)).xyz();
        }

        void transform(const Matrix4x4d& m)
        {
            v0 = convert_to<float>((m * Vector4d(convert_to<double>(v0), 1.0)).xyz());
            v1 = convert_to<float>((m * Vector4d(convert_to<double>(v1), 1.0)).xyz());
            v2 = convert_to<float>((m * Vector4d(convert_to<double>(v2), 1.0)).xyz());
        }

        Vector3f v0;
        Vector3f v1;
        Vector3f v2;

    private:
        int type;
        int attributes;
        int unused;

        // => 48 bytes
    };
}

#endif
