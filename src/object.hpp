#ifndef _dn_object_hpp_
#define _dn_object_hpp_

#include "dndefs.hpp"
#include "matrix4x4.hpp"
#include <boost/smart_ptr.hpp>

namespace dn
{
    class Object
    {
    public:
        virtual ~Object();

        virtual void done();
        virtual int get_primitive_count()=0;
        virtual void get_primitives(Primitive*)=0;
        virtual void get_primitive_materials(Material**)=0;

        virtual Vector3f calculate_normal(int id, float u, float v) const;

#ifdef DN_GL
        virtual void render_gl();
#endif

    protected:
        Object();
    };
}

#endif
