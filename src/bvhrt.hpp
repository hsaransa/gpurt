#ifndef _dn_bvhrt_hpp_
#define _dn_bvhrt_hpp_

#include "dndefs.hpp"
#include "aabb.hpp"
#include "primitive.hpp"
#include <vector>

namespace dn
{
    class BVHRT
    {
    public:
        class Node
        {
        public:
            Node() { left = right = 0; }
            ~Node() { delete left; delete right; }
            bool is_leaf() const { assert((left == 0) == (right == 0)); return left == 0; }
            void check() const;
            int count() const;
            int count_leaves() const;
            int count_inners() const;
            int primitive_max() const;
            double calculate_sah_cost() const;

            AABBf aabb;
            std::vector<int> primitives;
            Node* left;
            Node* right;
        };

        struct Intersection
        {
            int id;
            float t;
            float u;
            float v;

            int get_id() const { return id; }
            float get_t() const { return t; }
            float get_u() const { return u; }
            float get_v() const { return v; }
        };

        BVHRT(const Primitive* prims, int n);
        ~BVHRT();

        int intersect(const Vector3f& o, const Vector3f& d, float& t, float& u, float& v);

        Intersection intersect(const Vector3f& o, const Vector3f& d);

        Node* get_root() { return root; }
        int get_primitive_count() { return primitive_count; }
        const Primitive& get_primitive(int i) const { return primitives[i]; }

        int get_node_count() const { return root->count(); }
        int get_leaf_count() const { return root->count_leaves(); }
        int get_inner_count() const { return root->count_inners(); }
        int get_primitive_max() const { return root->primitive_max(); }

    private:
        void build(const Primitive* prims, int n);
        Node* build(int* prims, int n);
        Node* build_leaf(int* prims, int n);

        int primitive_count;
        const Primitive* primitives;
        Node* root;

        AABBf* aabbs;
    };
}

#endif
