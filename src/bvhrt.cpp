#include "bvhrt.hpp"
#include "primitive.hpp"
#include <stack>
#include <stdio.h>

using namespace dn;

BVHRT::BVHRT(const Primitive* prims, int n)
{
    root = 0;
    build(prims, n);
    root->check();
}

BVHRT::~BVHRT()
{
    delete root;
}

void BVHRT::build(const Primitive* prims, int n)
{
    primitives = prims;
    primitive_count = n;

    aabbs = new AABBf[primitive_count];
    for (int i = 0; i < primitive_count; i++)
        aabbs[i] = prims[i].get_aabb();

    int* indices = new int [primitive_count];
    for (int i = 0; i < primitive_count; i++)
        indices[i] = i;

    root = build(indices, primitive_count);

    delete [] indices;
    delete [] aabbs;
}

struct Sorter
{
    int axis;
    AABBf* aabbs;

    bool operator()(int i, int j)
    {
        float a = aabbs[i].min[axis] * .5f + aabbs[i].max[axis] * .5f;
        float b = aabbs[j].min[axis] * .5f + aabbs[j].max[axis] * .5f;
        return a < b;
    }
};

BVHRT::Node* BVHRT::build(int* prims, int n)
{
    if (n <= 3)
        return build_leaf(prims, n);

    AABBf aabb;
    for (int i = 0; i < n; i++)
        aabb.grow(aabbs[prims[i]]);

    float min_cost = aabb.get_surface_area() * n;
    int min_cost_axis = -1;
    int min_cost_pos = -1;

    for (int axis = 0; axis < 3; axis++)
    {
        Sorter sorter;
        sorter.axis = axis;
        sorter.aabbs = aabbs;
        std::sort(prims, prims+n, sorter);

        float* left_cost = new float [n];
        float* right_cost = new float [n];

        AABBf left_aabb;
        AABBf right_aabb;

        for (int i = 0; i < n; i++)
        {
            left_aabb.grow(aabbs[prims[i]]);
            left_cost[i] = left_aabb.get_surface_area() * (i+1);

            right_aabb.grow(aabbs[prims[n-i-1]]);
            right_cost[n-i-1] = right_aabb.get_surface_area() * (i+1);
        }

        for (int i = 1; i < n; i++)
        {
            if (left_cost[i-1] + right_cost[i] < min_cost)
            {
                min_cost = left_cost[i-1] + right_cost[i];
                min_cost_axis = axis;
                min_cost_pos = i;
            }
        }

        delete [] left_cost;
        delete [] right_cost;
    }

    if (min_cost_axis < 0)
        return build_leaf(prims, n);

    Sorter sorter;
    sorter.axis = min_cost_axis;
    sorter.aabbs = aabbs;
    std::sort(prims, prims+n, sorter);

    BVHRT::Node* node = new Node();
    node->aabb = aabb;
    node->left = build(prims, min_cost_pos);
    node->right = build(prims + min_cost_pos, n - min_cost_pos);

    return node;
}

BVHRT::Node* BVHRT::build_leaf(int* prims, int n)
{
    BVHRT::Node* node = new Node();

    for (int i = 0; i < n; i++)
        node->aabb.grow(aabbs[prims[i]]);

    node->left = 0;
    node->right = 0;

    for (int i = 0; i < n; i++)
        node->primitives.push_back(prims[i]);

    return node;
}

static bool intersects(const Vector3f& o, const Vector3f& d, const AABBf& aabb)
{
    float tmin = 0.f;
    float tmax = boost::numeric::bounds<float>::highest();

    for (int i = 0; i < 3; i++)
    {
        float t0 = (aabb.min[i] - o[i]) / d[i];
        float t1 = (aabb.max[i] - o[i]) / d[i];
        if (t1 < t0)
            std::swap(t0, t1);
        if (t0 > tmin)
            tmin = t0;
        if (t1 < tmax)
            tmax = t1;
    }

    return tmin <= tmax;
}

int BVHRT::intersect(const Vector3f& o, const Vector3f& d, float& t, float& u, float& v)
{
    int ni = -1;

    std::stack<Node*> st;
    st.push(root);

    int nodes_visited = 0;

    while (!st.empty())
    {
        Node* node = st.top();
        st.pop();

        nodes_visited++;

        if (!intersects(o, d, node->aabb))
            continue;

        if (node->left)
            st.push(node->left);
        if (node->right)
            st.push(node->right);

        for (int i = 0; i < (int)node->primitives.size(); i++)
        {
            const Primitive& prim = primitives[node->primitives[i]];
            float tt, uu, vv;
            if (prim.intersect(o, d, tt, uu, vv) && (ni == -1 || tt < t))
            {
                t = tt;
                u = uu;
                v = vv;
                ni = node->primitives[i];
            }
        }
    }

    //return nodes_visited;

    return ni;
}

BVHRT::Intersection BVHRT::intersect(const Vector3f& o, const Vector3f& d)
{
    Intersection is;
    is.id = intersect(o, d, is.t, is.u, is.v);
    return is;
}

void BVHRT::Node::check() const
{
    assert((left == 0) == (right == 0));
    assert((!left && !right) || primitives.empty());

    if (left)
        left->check();
    if (right)
        right->check();
}

int BVHRT::Node::count() const
{
    return 1 + (left ? left->count() : 0) + (right ? right->count() : 0);
}

int BVHRT::Node::count_leaves() const
{
    return
        (left == 0 && right == 0) +
        (left ? left->count_leaves() : 0) +
        (right ? right->count_leaves() : 0);
}

int BVHRT::Node::count_inners() const
{
    return
        (left || right) +
        (left ? left->count_inners() : 0) +
        (right ? right->count_inners() : 0);
}

int BVHRT::Node::primitive_max() const
{
    return std::max((int)primitives.size(),
               std::max(left ? left->primitive_max() : 0,
                        right ? right->primitive_max() : 0));
}

double BVHRT::Node::calculate_sah_cost() const
{
#if 0
    if (left || right)
    {
        return aabb.get_surface_area();
    }
    else
        return 
#endif

}
