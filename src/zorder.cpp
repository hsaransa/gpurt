#include "zorder.hpp"
#include "vector2.hpp"
#include "hostmemory.hpp"
#include <stdio.h>

using namespace dn;

ZOrder::ZOrder(int w, int h)
{
    assert(w && h); // TODO: fix, this is not an error

    // Next power of two.

    int w2 = w - 1;
    w2 |= w2 >> 1;
    w2 |= w2 >> 2;
    w2 |= w2 >> 4;
    w2 |= w2 >> 8;
    w2 |= w2 >> 16;
    w2++;

    int h2 = h - 1;
    h2 |= h2 >> 1;
    h2 |= h2 >> 2;
    h2 |= h2 >> 4;
    h2 |= h2 >> 8;
    h2 |= h2 >> 16;
    h2++;

    std::vector<Vector2i> vec_to_coord;
    std::vector<int> vec_to_index;
    vec_to_coord.resize(w*h);
    vec_to_index.resize(w*h);

    int idx = 0;
    for (int i = 0; i < w2*h2; i++)
    {
        int x = 0, y = 0;
        for (int j = 0; j < 16; j++)
        {
            x |= (i >> j) & (1 << j);
            y |= (i >> (j+1)) & (1 << j);
        }

        if (x < w && y < h)
            vec_to_index[y*w + x] = idx++;
    }

    to_index = new HostMemory(w*h*sizeof(int));
    to_index->fill(vec_to_index);

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            vec_to_coord[vec_to_index[y*w + x]] = Vector2i(x, y);

    to_coord = new HostMemory(w*h*2*sizeof(int));
    to_coord->fill(vec_to_coord);

#ifndef NDEBUG
    for (int i = 0; i < w*h; i++)
    {
        Vector2i p = vec_to_coord[i];
        assert(p.x >= 0 && p.x < w);
        assert(p.y >= 0 && p.y < h);
    }
#endif
}

ZOrder::~ZOrder()
{
}

HostMemory* ZOrder::get_to_index()
{
    return to_index;
}

HostMemory* ZOrder::get_to_coord()
{
    return to_coord;
}
