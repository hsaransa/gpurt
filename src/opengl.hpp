#ifndef _dn_opengl_hpp_
#define _dn_opengl_hpp_

#include "dndefs.hpp"
#ifdef DN_SDL
#   include "SDL_opengl.h"
#else
#endif

namespace dn
{
    class GLBufferObject
    {
    public:
        GLBufferObject();
        ~GLBufferObject();

    private:
    };
}

#endif
