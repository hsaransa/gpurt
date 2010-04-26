#include "objloader.hpp"
#include <string.h>
#include <stdio.h>

using namespace dn;

namespace
{
    char* dirname(char* p)
    {
        char* e = p + strlen(p) - 1;

        while (p < e && *e != '/')
        {
            *e = '\0';
            e--;
        }

        if (*e != '/')
            e[0] = '\0';

        return p;
    }

    std::string path_concat(const char* path, const char* filename)
    {
        char buf[256];
        assert(strlen(path) + strlen(filename) + 1 < sizeof(buf));
        strcpy(buf, path);
        dirname(buf);
        strcat(buf, filename);
        return buf;
    }
}

ObjLoader::ObjLoader(const char* filename, double s)
{
	load(filename);
	scale(Vector3d(s, s, s));
}

ObjLoader::ObjLoader(const char* filename, const Vector3d& s)
{
	load(filename);
	scale(s);
}

ObjLoader::~ObjLoader()
{
}

static char* strip_line(char* p)
{
	if (strchr(p, '#'))
		*strchr(p, '#') = '\0';
	if (strchr(p, '\r'))
		*strchr(p, '\r') = '\0';
	if (strchr(p, '\n'))
		*strchr(p, '\n') = '\0';

	while (isspace(*p))
		p++;

	return p;
}

void ObjLoader::load(const char* filename)
{
	FILE* fp = fopen(filename, "rt");
	if (!fp)
        throw std::runtime_error(std::string("can't open: ") + filename);

	int lineno = 0;
	int material = -1;
	int unknowns = 0;

	for (;;)
	{
		char buf[2048];
		if (fgets(buf, sizeof(buf), fp) == NULL)
			break;

		lineno++;

		char* p = strip_line(buf);

		if (*p == '\0')
			continue;

		if (strncmp(p, "v ", 2) == 0)
		{
			Vector3d v;
			sscanf(p+2, "%lf %lf %lf", &v.x, &v.y, &v.z);
			vertices.push_back(v);
		}
		else if (strncmp(p, "vn ", 3) == 0)
		{
			Vector3f n;
			sscanf(p+3, "%f %f %f", &n.x, &n.y, &n.z);
			normals.push_back(n);
		}
		else if (strncmp(p, "vt ", 3) == 0)
		{
			Vector3f t;
			sscanf(p+3, "%f %f %f", &t.x, &t.y, &t.z);
			textureCoords.push_back(t);
		}
		else if (strncmp(p, "f ", 2) == 0)
		{
			Polygon poly;
			char buf2[256] = "";
			int n;

			p += 2;

			int cnt = 0;
			while (sscanf(p, "%s%n", buf2, &n) != EOF)
			{
				VertexIndices vi;

				vi.v = -1;
				vi.t = -1;
				vi.n = -1;

				sscanf(buf2, "%d//", &vi.v);
				sscanf(buf2, "%d//%d", &vi.v, &vi.n);
				sscanf(buf2, "%d/%d/%d", &vi.v, &vi.t, &vi.n);

				if (vi.t < 0 && vi.t != -1)
                    fprintf(stderr, "TODO: fix, vi.t < 0 in sponsa.obj\n");

				vi.v = (vi.v <= 0) ? -1 : (vi.v - 1);
				//vi.t = (vi.t <= 0) ? -1 : (vi.t - 1);
				vi.t = (vi.t <= 0) ? (cnt % 4): (vi.t - 1);	// [samuli] hack for maze
				vi.n = (vi.n <= 0) ? -1 : (vi.n - 1);

				poly.vertices.push_back(vi);

				p += n;
				cnt++;
			}

			poly.material = material;
			polygons.push_back(poly);
		}
		else if (strncmp(p, "mtllib ", 7) == 0)
		{
			loadMtl(path_concat(filename, p+7).c_str());
		}
		else if (strncmp(p, "usemtl ", 7) == 0)
		{
			std::string s = p+7;

			unsigned int i;
			for (i = 0; i < materials.size(); i++)
			{
				if (materials[i].name == s)
					break;
			}

			material = (i == materials.size()) ? -1 : (int)i;
		}
		else
		{
			if (unknowns < 10)
				printf("%s: Unknown line %d: '%s'\n", filename, lineno, p);
			unknowns++;
		}
	}

	if (textureCoords.empty())
	{
		textureCoords.push_back(Vector3f(0, 0, 0));
		textureCoords.push_back(Vector3f(1, 0, 0));
		textureCoords.push_back(Vector3f(1, 1, 0));
		textureCoords.push_back(Vector3f(0, 1, 0));
	}

	fclose(fp);

	/* Check. */

	for (unsigned int i = 0; i < polygons.size(); i++)
	{
		const Polygon& p = polygons[i];

		for (unsigned int j = 0; j < p.vertices.size(); j++)
		{
			assert(p.vertices[j].v >= 0 && p.vertices[j].v < (int)vertices.size());
			assert(p.vertices[j].n < (int)normals.size());
			assert(p.vertices[j].t < (int)textureCoords.size());
		}
	}
}

void ObjLoader::loadMtl(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (!fp)
		return;

	int lineno = 0;
	int unknowns = 0;
	Material* mtl = NULL;

	for (;;)
	{
		char buf[2048];
		if (fgets(buf, sizeof(buf), fp) == NULL)
			break;

		lineno++;

		char* p = strip_line(buf);

		if (*p == '\0')
			continue;

		if (strncmp(p, "newmtl ", 7) == 0)
		{
			materials.push_back(Material());
			mtl = &materials[materials.size()-1];
			mtl->name = p+7;
		}
		else if (strncmp(p, "Ns ", 3) == 0)
		{
			if (mtl)
				sscanf(p+3, "%f", &mtl->shininess);
		}
		else if (strncmp(p, "d ", 2) == 0)
		{
			if (mtl)
				sscanf(p+2, "%f", &mtl->d);
		}
		else if (strncmp(p, "Kd ", 3) == 0)
		{
			if (mtl)
				sscanf(p+3, "%f %f %f", &mtl->diffuse[0], &mtl->diffuse[1], &mtl->diffuse[2]);
		}
		else if (strncmp(p, "Ka ", 3) == 0)
		{
			if (mtl)
				sscanf(p+3, "%f %f %f", &mtl->ambient[0], &mtl->ambient[1], &mtl->ambient[2]);
		}
		else if (strncmp(p, "Ks ", 3) == 0)
		{
			if (mtl)
				sscanf(p+3, "%f %f %f", &mtl->specular[0], &mtl->specular[1], &mtl->specular[2]);
		}
		else if (strncmp(p, "Ke ", 3) == 0)
		{
			if (mtl)
				sscanf(p+3, "%f %f %f", &mtl->emission[0], &mtl->emission[1], &mtl->emission[2]);
		}
		else if (strncmp(p, "map_Kd ", 7) == 0)
		{
			if (mtl)
				mtl->diffuseMap = path_concat(filename, p + 7);
		}
		else if (strncmp(p, "map_Bump ", 9) == 0)
		{
			if (mtl)
				mtl->bumpMap = path_concat(filename, p + 9);
		}
		else
		{
			if (unknowns < 10)
				printf("%s: Unknown line %d: '%s'\n", filename, lineno, p);
			unknowns++;
		}
	}

	fclose(fp);

	/* TODO: check duplicates */
}

void ObjLoader::scale(const Vector3d& s)
{
	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices[i] = dn::scale(vertices[i], s);
}
