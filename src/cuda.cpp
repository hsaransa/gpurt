#include "cuda.hpp"
#include "vector4.hpp"
#include "hostmemory.hpp"
#include <cuda.h>
#include <stdexcept>
#include <cassert>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

using namespace dn;

static void error(CUresult res, const char* s)
{
    std::ostringstream ss;
    ss << "CUDA error: " << s << ": " << res;
    throw std::runtime_error(ss.str());
}

static CUdevice device;
static CUcontext context;
static std::map<std::string, CUmodule> name_to_module;
static std::vector<CUmodule> modules;

#define SAFE(x) { CUresult res = x; if (res != CUDA_SUCCESS) error(res, #x); }

void dn::cuda_init()
{
    SAFE(cuInit(0));

    int cnt;
    SAFE(cuDeviceGetCount(&cnt));
    assert(cnt);

    SAFE(cuDeviceGet(&device, 0));

    SAFE(cuCtxCreate(&context, CU_CTX_MAP_HOST, device));
}

void dn::cuda_print_info()
{
    char name[128];
    SAFE(cuDeviceGetName(name, sizeof(name), device));
    fprintf(stderr, "CUDA device 0:      %s\n", name);
    fprintf(stderr, "CUDA compute cap:   %d\n", cuda_ver());
    fprintf(stderr, "Clock:              %.2f MHz\n", cuda_attribute(CU_DEVICE_ATTRIBUTE_CLOCK_RATE) / 1000.0);
    fprintf(stderr, "Memory:             %.2f MB\n", cuda_total_memory() / 1024.0 / 1024.0);
    fprintf(stderr, "Shared mem:         %d B\n", cuda_attribute(CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK));
    fprintf(stderr, "Constant mem:       %d B\n", cuda_attribute(CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY));
    fprintf(stderr, "Warp width:         %d\n", cuda_attribute(CU_DEVICE_ATTRIBUTE_WARP_SIZE));
    fprintf(stderr, "GPU overlap:        %d\n", cuda_attribute(CU_DEVICE_ATTRIBUTE_GPU_OVERLAP));
    fprintf(stderr, "SM count:           %d\n", cuda_attribute(CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT));
    fprintf(stderr, "Registers in block: %d\n", cuda_attribute(CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK));
    fprintf(stderr, "Threads in block:   %d\n", cuda_attribute(CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK));
}

unsigned int dn::cuda_total_memory()
{
    unsigned int mem;
    SAFE(cuDeviceTotalMem(&mem, device));
    return mem;
}

int dn::cuda_ver()
{
    int major;
    int minor;
    SAFE(cuDeviceComputeCapability(&major, &minor, device));
    return major * 10 + minor;
}

int dn::cuda_attribute(CUdevice_attribute attr)
{
    int v;
    SAFE(cuDeviceGetAttribute(&v, attr, device));
    return v;
}

void dn::cuda_sync()
{
    SAFE(cuCtxSynchronize());
}

unsigned int dn::cuda_alloc(unsigned int size)
{
    CUdeviceptr ptr;
    SAFE(cuMemAlloc(&ptr, size));
    return ptr;
}

void dn::cuda_free(unsigned int ptr)
{
    SAFE(cuMemFree((unsigned int)ptr));
}

void dn::cuda_memcpy_h2d(unsigned int dst, const void* src, unsigned int size)
{
    SAFE(cuMemcpyHtoD(dst, src, size));
}

void dn::cuda_memcpy_d2h(void* dst, unsigned int src, unsigned int size)
{
    SAFE(cuMemcpyDtoH(dst, src, size));
}

/*
 * CudaMemory
 */

CudaMemory::CudaMemory(unsigned int nsize, bool in_host)
{
    this->in_host = in_host;
    size = 0;
    host_ptr = 0;
    dev_ptr = 0;
    if (nsize)
        resize(nsize);
}

CudaMemory::CudaMemory(HostMemory* m)
{
    in_host = false;
    size = 0;
    host_ptr = 0;
    dev_ptr = 0;
    resize(m->get_size());
    write(m);
}

CudaMemory::~CudaMemory()
{
    resize(0);
}

void CudaMemory::resize(unsigned int nsize)
{
    if (size == nsize)
        return;

    if (in_host)
    {
        if (host_ptr)
            cuMemFreeHost(host_ptr);
    }
    else
    {
        if (dev_ptr)
            cuMemFree(dev_ptr);
    }

    size = nsize;
    host_ptr = 0;
    dev_ptr = 0;

    if (size)
    {
        if (in_host)
        {
            SAFE(cuMemHostAlloc(&host_ptr, size, CU_MEMHOSTALLOC_DEVICEMAP));
            SAFE(cuMemHostGetDevicePointer(&dev_ptr, host_ptr, 0));
        }
        else
        {
            cuMemAlloc(&dev_ptr, size);
        }
    }
}

unsigned int CudaMemory::get_size()
{
    return size;
}

void* CudaMemory::get_host_ptr()
{
    assert(host_ptr);
    return host_ptr;
}

CUdeviceptr CudaMemory::get_device_ptr()
{
    assert((dev_ptr == 0) == (size == 0));
    return dev_ptr;
}

void CudaMemory::read(HostMemory* m)
{
    assert(m->get_size() <= get_size());
    unsigned int count = std::min(m->get_size(), get_size());
    SAFE(cuMemcpyDtoH(m->get_ptr(), get_device_ptr(), count));
}

void CudaMemory::write(HostMemory* m)
{
    assert(get_size() <= m->get_size());
    unsigned int count = std::min(m->get_size(), get_size());
    SAFE(cuMemcpyHtoD(get_device_ptr(), m->get_ptr(), count));
}

void CudaMemory::set(int v)
{
    SAFE(cuMemsetD8(get_device_ptr(), v, get_size()));
}

/*
 * CudaStream
 */

CudaStream::CudaStream()
{
    SAFE(cuStreamCreate(&stream, 0));
}

CudaStream::~CudaStream()
{
    SAFE(cuStreamSynchronize(stream));
    SAFE(cuStreamDestroy(stream));
}

bool CudaStream::ready()
{
    CUresult res = cuStreamQuery(stream);
    assert(res == CUDA_ERROR_NOT_READY || res == CUDA_SUCCESS);
    return res == CUDA_SUCCESS;
}

/*
 * CudaModule
 */

CudaModule::CudaModule(const std::string& filename) : func(0)
{
    set_block_dim(32, 4);
    SAFE(cuModuleLoad(&module, filename.c_str()));
}

CudaModule::~CudaModule()
{
    SAFE(cuModuleUnload(module));
}

void CudaModule::set_block_dim(int w, int h)
{
    block_w = w;
    block_h = h;
}

void CudaModule::prepare_launch(const std::string& name)
{
    prepare_launch(name, "");
}

void CudaModule::prepare_launch(const std::string& name, const char* f, ...)
{
    SAFE(cuModuleGetFunction(&func, module, name.c_str()));

    SAFE(cuFuncSetBlockShape(func, block_w, block_h, 1));

    // Param size.

    int size = 0;

    for (const char* p = f; *p; p++)
    {
        switch (*p)
        {
        case 'i': size += 4; break;
        case 'f': size += 4; break;
        default: assert(0);
        }
    }

    // Set params.

    va_list args;
    va_start(args, f);
    int offset = 0;

    for (const char* p = f; *p; p++)
    {
        unsigned int ival;
        float fval;

        switch (*p)
        {
        case 'i':
            ival = va_arg(args, unsigned int);
            cuParamSeti(func, offset, ival);
            offset += 4;
            break;
        case 'f':
            fval = (float)va_arg(args, double);
            cuParamSetf(func, offset, fval);
            offset += 4;
            break;
        }
    }

    va_end(args);

    SAFE(cuParamSetSize(func, size));
}

void CudaModule::launch(CudaStream* stream, int w, int h)
{
    SAFE(cuLaunchGridAsync(func, w, h, (CUstream)stream->stream));
}

int CudaModule::get_int(const std::string& var)
{
    CUdeviceptr ptr;
    SAFE(cuModuleGetGlobal(&ptr, 0, module, var.c_str()));

    int value;
    cuda_memcpy_d2h(&value, ptr, 4);

    return value;
}

void CudaModule::set_int(const std::string& var, int val)
{
    CUdeviceptr ptr;
    SAFE(cuModuleGetGlobal(&ptr, 0, module, var.c_str()));
    cuda_memcpy_h2d(ptr, &val, 4);
}

void CudaModule::set_float4(const std::string& var, const Vector4f& v)
{
    CUdeviceptr ptr;
    SAFE(cuModuleGetGlobal(&ptr, 0, module, var.c_str()));
    cuda_memcpy_h2d(ptr, &v.x, 16);
}

void CudaModule::set_ptr(const std::string& var, CudaMemory* m)
{
    CUdeviceptr ptr;
    CUdeviceptr value = m->get_device_ptr();
    SAFE(cuModuleGetGlobal(&ptr, 0, module, var.c_str()));
    cuda_memcpy_h2d(ptr, &value, sizeof(CUdeviceptr));
}

CudaTexture* CudaModule::get_texture(const std::string& name)
{
    assert(func);
    CUtexref tex;
    SAFE(cuModuleGetTexRef(&tex, module, name.c_str()));
    return new CudaTexture(func, tex);
}

/*
 * CudaEvent
 */

CudaEvent::CudaEvent()
{
    SAFE(cuEventCreate(&event, CU_EVENT_DEFAULT));
}

CudaEvent::~CudaEvent()
{
    SAFE(cuEventDestroy(event));
}

void CudaEvent::record()
{
    SAFE(cuEventRecord(event, 0));
}

void CudaEvent::record(CudaStream* stream)
{
    SAFE(cuEventRecord(event, stream->stream));
}

float CudaEvent::elapsed_time(CudaEvent* ev)
{
    ev->sync();
    sync();
    float f;
    SAFE(cuEventElapsedTime(&f, ev->event, event));
    return f;
}

bool CudaEvent::ready()
{
    CUresult res = cuEventQuery(event);
    assert(res == CUDA_ERROR_NOT_READY || res == CUDA_SUCCESS);
    return res == CUDA_SUCCESS;
}

void CudaEvent::sync()
{
    SAFE(cuEventSynchronize(event));
}

/*
 * CudaTexture
 */

#if 0
CudaTexture::CudaTexture()
:   owned(true)
{
    SAFE(cuTexRefCreate(&texref));
    SAFE(cuTexRefSetFilterMode(texref, CU_TR_FILTER_MODE_POINT));
    SAFE(cuTexRefSetAddressMode(texref, 1, CU_TR_ADDRESS_MODE_CLAMP));
    SAFE(cuTexRefSetFlags(texref, CU_TRSF_READ_AS_INTEGER));
}
#endif

CudaTexture::CudaTexture(CUfunction func, CUtexref tex)
:   owned(false), func(func), texref(tex)
{
    SAFE(cuTexRefSetFilterMode(texref, CU_TR_FILTER_MODE_POINT));
    SAFE(cuTexRefSetAddressMode(texref, 1, CU_TR_ADDRESS_MODE_CLAMP));
    SAFE(cuTexRefSetFlags(texref, CU_TRSF_READ_AS_INTEGER));
}

CudaTexture::~CudaTexture()
{
    if (owned)
        SAFE(cuTexRefDestroy(texref));
}

void CudaTexture::set(CudaMemory* mem, Format f, int c)
{
    assert(mem);
    assert(f >= 0 && f <= FLOAT);
    assert(c >= 1 && c <= 4);

    unsigned int offset;
    SAFE(cuTexRefSetAddress(&offset, texref,
        mem->get_device_ptr(), mem->get_size()));
    assert(offset == 0);

    CUarray_format cf = CU_AD_FORMAT_UNSIGNED_INT8;

    switch (f)
    {
    case UINT8: cf = CU_AD_FORMAT_UNSIGNED_INT8; break;
    case UINT16: cf = CU_AD_FORMAT_UNSIGNED_INT16; break;
    case UINT32: cf = CU_AD_FORMAT_UNSIGNED_INT32; break;
    case INT8: cf = CU_AD_FORMAT_SIGNED_INT8; break;
    case INT16: cf = CU_AD_FORMAT_SIGNED_INT16; break;
    case INT32: cf = CU_AD_FORMAT_SIGNED_INT32; break;
    case HALF: cf = CU_AD_FORMAT_HALF; break;
    case FLOAT: cf = CU_AD_FORMAT_FLOAT; break;
    }

    SAFE(cuTexRefSetFormat(texref, cf, c));
    SAFE(cuTexRefSetFilterMode(texref, CU_TR_FILTER_MODE_POINT));
    SAFE(cuTexRefSetAddressMode(texref, 1, CU_TR_ADDRESS_MODE_CLAMP));
    SAFE(cuTexRefSetFlags(texref, CU_TRSF_READ_AS_INTEGER));

    SAFE(cuParamSetTexRef(func, CU_PARAM_TR_DEFAULT, texref));
}
