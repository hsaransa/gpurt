#ifndef _dn_cuda_hpp_
#define _dn_cuda_hpp_

#include "dndefs.hpp"
#include <cuda.h>
#include <string>
#include <vector>
#include <iostream>

namespace dn
{
    void cuda_init();
    void cuda_print_info();
    int cuda_ver();
    unsigned int cuda_total_memory();
    int cuda_attribute(CUdevice_attribute attr);
    void cuda_sync();

    unsigned int cuda_alloc(unsigned int size);
    void cuda_free(unsigned int ptr);

    void cuda_memcpy_h2d(unsigned int, const void*, unsigned int size);
    void cuda_memcpy_d2h(void*, unsigned int, unsigned int size);

    class CudaMemory
    {
    public:
        CudaMemory(unsigned int size=0, bool in_host=false);
        CudaMemory(HostMemory* m);
        ~CudaMemory();

        void resize(unsigned int size);
        unsigned int get_size();

        void* get_host_ptr();
        CUdeviceptr get_device_ptr();

        void set(int v);

        void read(HostMemory* m);
        void write(HostMemory* m);

        template<typename T>
        void fill(const std::vector<T>& v)
        {
            assert((sizeof(T) % 4) == 0); // TODO: better assert
            resize(v.size() * sizeof(T));
            cuda_memcpy_h2d(get_device_ptr(), &v[0], sizeof(T) * v.size());
        }

    private:
        bool in_host;
        unsigned int size;
        void* host_ptr;
        CUdeviceptr dev_ptr;
    };

    class CudaStream
    {
        friend class CudaModule;
        friend class CudaEvent;
    public:
        CudaStream();
        ~CudaStream();

        bool ready();

    private:
        CUstream stream;
    };

    class CudaModule
    {
    public:
        CudaModule(const std::string& filename);
        ~CudaModule();

        void set_block_dim(int w, int h);
        int get_block_width() { return block_w; }
        int get_block_height() { return block_h; }

        void prepare_launch(const std::string& name);
        void prepare_launch(const std::string& name, const char* f, ...);
        void launch(CudaStream* stream, int w, int h);

        int get_int(const std::string& var);
        void set_int(const std::string& var, int val);
        void set_float4(const std::string& var, const Vector4f& v);
        void set_ptr(const std::string& var, CudaMemory*);

        CudaTexture* get_texture(const std::string& name);

    private:
        int block_w, block_h;
        CUmodule module;
        CUfunction func;
    };

    class CudaEvent
    {
    public:
        CudaEvent();
        ~CudaEvent();

        void record();
        void record(CudaStream* stream);
        bool ready();

        float elapsed_time(CudaEvent* start);

        void sync();

    private:
        CUevent event;
    };

    class CudaTexture
    {
    public:
        enum Format
        {
            UINT8, UINT16, UINT32,
            INT8, INT16, INT32,
            HALF, FLOAT
        };

        CudaTexture(CUfunction, CUtexref);
        ~CudaTexture();

        void set(CudaMemory* mem, Format f, int c);

    private:
        bool owned;
        CUfunction func;
        CUtexref texref;
    };

    class CudaMeasureTime
    {
    public:
        CudaMeasureTime()
        {
            start();
        }

        void start()
        {
            startev.record();
        }

        float measure(CudaStream* stream)
        {
            end.record(stream);
            return end.elapsed_time(&startev);
        }

    public:
        CudaEvent startev, end;
    };
}

#endif
