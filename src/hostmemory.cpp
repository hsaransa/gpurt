#include "hostmemory.hpp"
#include <stdexcept>
#include <stdio.h>

using namespace dn;

/*
 * HostMemory
 */

HostMemory::HostMemory()
:   size(0), ptr(0)
{
}

HostMemory::HostMemory(unsigned int s)
:   size(0), ptr(0)
{
    resize(s);
}

HostMemory::~HostMemory()
{
    free(ptr);
}

void HostMemory::resize(unsigned int size)
{
    void* nptr = realloc(ptr, size);
    if (!nptr)
        throw std::runtime_error("can't allocate host memory");
    this->ptr = nptr;
    this->size = size;
}

unsigned int HostMemory::get_size()
{
    return size;
}

void* HostMemory::get_ptr()
{
    return ptr;
}

/*
 * DiskMemory
 */

DiskMemory::DiskMemory(const std::string& name)
:   name(name)
{
}

DiskMemory::DiskMemory(const std::string& name, HostMemory* m)
:   name(name)
{
    FILE* fp = fopen(get_filename().c_str(), "wb");
    if (!fp)
        throw std::runtime_error("can't open " + get_filename() + " for writing");
    fwrite(m->get_ptr(), 1, m->get_size(), fp);
    fclose(fp);
}

DiskMemory::~DiskMemory()
{
}

std::string DiskMemory::get_filename() const
{
    return "cache-" + name + ".bin";
}

unsigned int DiskMemory::get_size()
{
    FILE* fp = fopen(get_filename().c_str(), "rb");
    if (!fp)
        return 0;
    fseek(fp, 0, SEEK_END);
    int l = ftell(fp);
    assert(l >= 0);
    fclose(fp);
    return l;
}

HostMemory* DiskMemory::get_host_memory()
{
    unsigned int s = get_size();
    HostMemory* m = new HostMemory(s);
    if (s == 0)
        return m;

    FILE* fp = fopen(get_filename().c_str(), "rb");
    if (!fp)
        throw std::runtime_error("can't open " + get_filename() + " for reading");
    fread(m->get_ptr(), 1, s, fp);
    fclose(fp);
    return m;
}
