#ifndef _dn_material_hpp_
#define _dn_material_hpp_

#include "dndefs.hpp"
#include "vector3.hpp"

namespace dn
{
    class Material
    {
    public:
        Material();
        ~Material();

        void set_light(bool l) { light = l; }
        void set_power(const Vector3f& p) { power = p; }

    public:
        bool light;
        Vector3f power;

        bool diffuse_enabled;
        Vector3f diffuse;
    };
}

#endif
