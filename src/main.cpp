#include "SDL.h"
#include <SDL_opengl.h>
#include "objloader.hpp"
#include "bvhrt.hpp"
#include "cudabvh.hpp"
#include "zorder.hpp"
#include "cuda.hpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define RENDER_WIDTH 128
#define RENDER_HEIGHT 128

using namespace dn;

static enum {
    RENDER_GL,
    RENDER_RT_CPU,
    RENDER_RT_CUDA,
} mode = RENDER_GL;

static Matrix4x4f cam_to_clip;
static Matrix4x4f cam_to_view;

static const float move_speed = 5.f;

static std::vector<Primitive> primitives;
static BVHRT* bvhrt;
static CudaBVH* cudabvh;
static ZOrder* zorder;
static CudaModule* module;
static CudaMemory* cuda_result;

static void init()
{
    cuda_init();
    cuda_print_info();

    fprintf(stderr, "loading model\n");

    ObjLoader obj("bunny.obj");

    for (int i = 0; i < (int)obj.polygons.size(); i++)
    {
        for (int j = 2; j < (int)obj.polygons[i].vertices.size(); j++)
        {
            Vector3i t;
            t.x = obj.polygons[i].vertices[0].v;
            t.y = obj.polygons[i].vertices[j-1].v;
            t.z = obj.polygons[i].vertices[j].v;

            Vector3d v0 = obj.vertices[t.x];
            Vector3d v1 = obj.vertices[t.y];
            Vector3d v2 = obj.vertices[t.z];

            // Remove degenerate triangles.
            if (cross(v1 - v0, v2 - v0).length() < 0.00001)
                continue;

            Primitive prim = Primitive(Primitive::TRIANGLE,
                    convert_to<float>(v0),
                    convert_to<float>(v1),
                    convert_to<float>(v2));

            primitives.push_back(prim);
        }
    }

    fprintf(stderr, "building bvh tree\n");

    bvhrt = new BVHRT(&*primitives.begin(), primitives.size());

    fprintf(stderr, "preparing cuda\n");

    module = new CudaModule("cudabvh.cubin");
    module->set_block_dim(32, 2);
    cudabvh = new CudaBVH(bvhrt);
    zorder = new ZOrder(RENDER_WIDTH, RENDER_HEIGHT);

    // Set pointers to data.

    module->set_ptr("nodes", cudabvh->get_cuda_nodes());
    module->set_ptr("aabbs_x", cudabvh->get_cuda_aabbs_x());
    module->set_ptr("aabbs_y", cudabvh->get_cuda_aabbs_y());
    module->set_ptr("aabbs_z", cudabvh->get_cuda_aabbs_z());
    module->set_ptr("vertices", cudabvh->get_cuda_vertices());

    // Set textures also. Kernel may use either one.

    // textures are actually bounded to functions so function must be prepared beforehand
    module->prepare_launch("bvh_trace");
    module->get_texture("tex_nodes")->set(cudabvh->get_cuda_nodes(), CudaTexture::INT32, 2);
    module->get_texture("tex_aabbs_x")->set(cudabvh->get_cuda_aabbs_x(), CudaTexture::FLOAT, 4);
    module->get_texture("tex_aabbs_y")->set(cudabvh->get_cuda_aabbs_y(), CudaTexture::FLOAT, 4);
    module->get_texture("tex_aabbs_z")->set(cudabvh->get_cuda_aabbs_z(), CudaTexture::FLOAT, 4);
    module->get_texture("tex_vertices")->set(cudabvh->get_cuda_vertices(), CudaTexture::FLOAT, 4);

    CudaMemory* mem = new CudaMemory(zorder->get_to_coord());
    module->set_ptr("zorder", mem);

    // Set destination buffer.

    cuda_result = new CudaMemory(RENDER_WIDTH * RENDER_HEIGHT * 4, true);
    module->set_ptr("result", cuda_result);
    module->set_int("width", RENDER_WIDTH);
    module->set_int("height", RENDER_HEIGHT);

    fprintf(stderr, "init done\n");
}

//
// Draws scene using OpenGL.
//

static void draw_gl()
{
    glClearColor(1.f, 0.f, 0.4f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(transpose(cam_to_clip).data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(transpose(cam_to_view).data());

    glColor3f(1.f, 1.f, 1.f);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < (int)primitives.size(); i++)
    {
        Vector3f n = normalize(primitives[i].get_normal(0.f, 0.f));
        n = n * 0.5f + Vector3f(0.5f, 0.5f, 0.5f);
        glColor3fv(&n.x);
        glVertex3fv(&primitives[i].v0.x);
        glVertex3fv(&primitives[i].v1.x);
        glVertex3fv(&primitives[i].v2.x);
    }
    glEnd();
}

//
// Draws scene using CPU ray tracing.
//

static void draw_rt_cpu()
{
    Matrix4x4f to_world = invert(cam_to_clip * cam_to_view);

    unsigned char* buf = (unsigned char*)malloc(RENDER_WIDTH * RENDER_HEIGHT * 4);
    unsigned char* p = buf;

    for (int y = 0; y < RENDER_HEIGHT; y++)
        for (int x = 0; x < RENDER_WIDTH; x++)
        {
            float fx = (x + 0.5f) / RENDER_WIDTH * 2.f - 1.f;
            float fy = (y + 0.5f) / RENDER_HEIGHT * 2.f - 1.f;

            Vector3f p0 = (to_world * Vector4f(fx, fy, -1.f, 1.f)).project();
            Vector3f p1 = (to_world * Vector4f(fx, fy, 1.f, 1.f)).project();

            float t, u, v;
            int ret = bvhrt->intersect(p0, p1 - p0, t, u, v);

            if (ret < 0)
            {
                *p++ = 0xFF;
                *p++ = 0x00;
                *p++ = 0xFF;
                *p++ = 0xFF;
                continue;
            }

            Vector3f n = normalize(primitives[ret].get_normal(0.f, 0.f));
            n = n * 0.5f + Vector3f(0.5f, 0.5f, 0.5f);

            *p++ = 0x00 + n.x * 255.f;
            *p++ = 0x00 + n.y * 255.f;
            *p++ = 0x00 + n.z * 255.f;
            *p++ = 0xFF;
        }

    glPixelZoom(WINDOW_WIDTH / (float)RENDER_WIDTH, WINDOW_HEIGHT / (float)RENDER_HEIGHT);
    glDrawPixels(RENDER_WIDTH, RENDER_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    free(buf);
}

//
// Draws scene using CUDA.
//

static void draw_rt_cuda()
{
    Matrix4x4f to_world = invert(cam_to_clip * cam_to_view);

    module->set_float4("matrix0", to_world.row(0));
    module->set_float4("matrix1", to_world.row(1));
    module->set_float4("matrix2", to_world.row(2));
    module->set_float4("matrix3", to_world.row(3));
    module->set_int("warp_counter", 0);

    module->prepare_launch("bvh_trace");

    CudaStream* strm = new CudaStream();
    module->launch(strm, 32, 32);
    delete strm;

    glDisable(GL_DEPTH_TEST);

    glPixelZoom(WINDOW_WIDTH / (float)RENDER_WIDTH, WINDOW_HEIGHT / (float)RENDER_HEIGHT);
    glDrawPixels(RENDER_WIDTH, RENDER_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, cuda_result->get_host_ptr());



}

static void move(const Vector3f& m)
{
    cam_to_view = translate(m) * cam_to_view;
}

int main()
{
    init();

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    atexit(SDL_Quit);

    if (SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_OPENGL) == 0)
    {
        fprintf(stderr, "SDL_SetVideoMode failed: %s\n", SDL_GetError());
        return 1;
    }

    cam_to_clip = perspective<float>(45.f / 180.f * 3.14159265f, 1.f, 0.1f, 100.f);

    Vector3f eye = Vector3f(4.f, 1.f, 4.f);
    Vector3f center = Vector3f(0.f, 0.f, 0.f);
    Vector3f up = Vector3f(0.f, 1.f, 0.f);
    cam_to_view = look_at<float>(eye, center, up);

    int prev_ticks = SDL_GetTicks();

    while (1)
    {
        int dt = SDL_GetTicks() - prev_ticks;
        prev_ticks += dt;

        // Handle events.

        SDL_Event ev;

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    return 0;

                case SDLK_1:
                    fprintf(stderr, "rendering using OpenGL\n");
                    mode = RENDER_GL;
                    break;

                case SDLK_2:
                    fprintf(stderr, "rendering using cpu ray tracing\n");
                    mode = RENDER_RT_CPU;
                    break;

                case SDLK_3:
                    fprintf(stderr, "rendering using cuda ray tracing\n");
                    mode = RENDER_RT_CUDA;
                    break;

                default:
                    break;
                }
                break;

            case SDL_QUIT:
                return 0;
            }
        }

        // Handle movement.

        float dtf = dt / 1000.f;

        Uint8* keys = SDL_GetKeyState(0);

        if (keys[SDLK_UP])
            move(Vector3f(0.f, 0.f, move_speed) * dtf);
        if (keys[SDLK_DOWN])
            move(Vector3f(0.f, 0.f, -move_speed) * dtf);
        if (keys[SDLK_LEFT])
            move(Vector3f(move_speed, 0.f, 0.f) * dtf);
        if (keys[SDLK_RIGHT])
            move(Vector3f(-move_speed, 0.f, 0.f) * dtf);

        // Draw things.

        switch (mode)
        {
        case RENDER_GL:
            draw_gl();
            break;
        case RENDER_RT_CPU:
            draw_rt_cpu();
            break;
        case RENDER_RT_CUDA:
            draw_rt_cuda();
            break;
        };

        SDL_GL_SwapBuffers();
    }

    return 0;
}
