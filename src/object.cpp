#include "object.hpp"
#include "aabb.hpp"
#if DN_GL
#   include "opengl.hpp"
#endif

using namespace dn;

Object::Object()
{
}

Object::~Object()
{
}

void Object::done()
{
}

Vector3f Object::calculate_normal(int id, float u, float v) const
{
    throw std::runtime_error("calculate_normal() requested but not implemented");
}

#ifdef DN_GL
void Object::render_gl()
{
#if 0
    AABBd aabb = get_aabb();
    if (!aabb.is_valid())
        return;

    Vector3f corners[8];

    for (int i = 0; i < 8; i++)
    {
        corners[i].x = float((i&1) ? aabb.min.x : aabb.max.x);
        corners[i].y = float((i&2) ? aabb.min.y : aabb.max.y);
        corners[i].z = float((i&4) ? aabb.min.z : aabb.max.z);
    }

    glColor3f(0.f, 0.f, 0.f);
    glLineWidth(2.f);
    glBegin(GL_LINES);

    for (int i = 0; i < 8; i++)
        for (int j = 1; j < 8; j *= 2)
        {
            if (i & j)
                continue;

            glVertex3fv(&corners[i].x);
            glVertex3fv(&corners[i|j].x);
        }

    glEnd();
#endif
}
#endif
