#ifndef _dn_objloader_hpp_
#define _dn_objloader_hpp_

#include "vector3.hpp"
#include <vector>

namespace dn
{
    class ObjLoader
    {
    public:
        struct VertexIndices
        {
            int v;
            int t;
            int n;
        };

        struct Material
        {
            std::string name;
            int illum;
            float ambient[3];
            std::string ambientMap;
            float diffuse[3];
            std::string diffuseMap;
            float specular[3];
            std::string specularMap;
            float emission[3];
            std::string emissionMap;
            std::string bumpMap;
            float shininess;
            float d; // What's this? Opacity?
        };

        class Polygon
        {
            public:
                int material;
                std::vector<VertexIndices> vertices;
        };

        ObjLoader(const char* filename, double scale = 1.0);
        ObjLoader(const char* filename, const Vector3d& s);
        ~ObjLoader();

        void translate(double x, double y, double z)
        {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i] += Vector3d(x, y, z);
        }

        void swapYZ()
        {
            for (unsigned int i = 0; i < vertices.size(); i++)
                std::swap(vertices[i].y, vertices[i].z);

            for (unsigned int i = 0; i < normals.size(); i++)
                std::swap(normals[i].y, normals[i].z);

            for (unsigned int i = 0; i < polygons.size(); i++)
                polygons[i].vertices = std::vector<VertexIndices>(polygons[i].vertices.rbegin(), polygons[i].vertices.rend());
        }

        void negateXZ()
        {
            for (unsigned int i = 0; i < vertices.size(); i++)
            {
                vertices[i].x *= -1.0;
                vertices[i].z *= -1.0;
            }

            for (unsigned int i = 0; i < normals.size(); i++)
            {
                normals[i].x *= -1.0f;
                normals[i].z *= -1.0f;
            }
        }

    public:
        std::vector<Vector3d> vertices;
        std::vector<Vector3f> normals;
        std::vector<Vector3f> textureCoords;
        std::vector<Polygon> polygons;
        std::vector<Material> materials;

    private:
        void load(const char* filename);
        void loadMtl(const char* filename);
        void scale(const Vector3d& s);
    };
}

#endif
