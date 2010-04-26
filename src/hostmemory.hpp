#ifndef _dn_hostmemory_hpp_
#define _dn_hostmemory_hpp_

#include "dndefs.hpp"
#include "cuda.hpp"
#include <string.h>
#include <vector>
#include <string>

namespace dn
{
    class HostMemory
    {
    public:
        HostMemory();
        HostMemory(unsigned int s);
        ~HostMemory();

        void resize(unsigned int size);

        unsigned int get_size();
        void* get_ptr();

        template<typename T>
        void fill(const std::vector<T>& v)
        {
            assert((sizeof(T) % 4) == 0); // TODO: assert sizeof(T)*v.size() == end() - begin() or something
            resize(v.size() * sizeof(T));
            memcpy(ptr, &v[0], sizeof(T) * v.size());
        }

    private:
        unsigned int size;
        void* ptr;
    };

    class DiskMemory
    {
    public:
        DiskMemory(const std::string& name);
        DiskMemory(const std::string& name, HostMemory* m);
        ~DiskMemory();

        std::string get_filename() const;
        unsigned int get_size();

        HostMemory* get_host_memory();

    private:
        std::string name;
    };
};

#endif
