#include "material.hpp"

using namespace dn;

Material::Material()
{
    light = false;

    diffuse_enabled = false;
    diffuse = Vector3f(0.7f, 0.7f, 0.7f);
}

Material::~Material()
{
}
