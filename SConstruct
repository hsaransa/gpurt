# gpurt scons file

CUDA_PATH = '/usr/local/cuda'

import glob, os
import SCons

env = Environment()

release  = int(ARGUMENTS.get('release', 0))
use_sdl  = True
use_gl   = True
use_cuda = True

cuda_regcount = int(ARGUMENTS.get('cuda_regcount', 23))

have_openexr = int(ARGUMENTS.get('have_openexr', 0))
have_pngwriter = int(ARGUMENTS.get('have_pngwriter', 0))

# Stuff

is_mac = os.uname()[0] == 'Darwin'

if release:
  env['OBJPREFIX'] += 'objs/'
else:
  env['OBJPREFIX'] += 'objsd/'

env.Append(CPPFLAGS=['-Wall'])

if not release:
  env.Append(CPPFLAGS=['-g', '-O1'])
else:
  env.Append(CPPDEFINES=[('NDEBUG')])
  env.Append(CPPFLAGS=['-g', '-O2', '-ffast-math', '-fomit-frame-pointer'])

if release:
  env.Append(LINKFLAGS=['-static-libgcc'])

#env.Append(CPPPATH=['../boost_1_41_0'])

if have_openexr:
  env.Append(CPPDEFINES=[('HAVE_OPENEXR', 1)])
  env.Append(CPPPATH=['/usr/include/OpenEXR'])
  env.Append(CPPPATH=['/usr/local/include/OpenEXR'])
  #env.Append(CPPPATH=['/sw/include/OpenEXR'])
  #env.Append(LIBPATH=['/sw/lib'])
  env.Append(LIBS=['Half'])
  env.Append(LIBS=['Iex'])
  env.Append(LIBS=['IlmImf'])

if have_pngwriter:
  env.Append(CPPDEFINES=[('HAVE_PNGWRITER', 1)])
  env.Append(LIBS=['pngwriter', 'freetype'])

# SDL

if use_sdl:
  env.Append(CPPPATH=['/usr/include/SDL'])
  env.Append(CPPPATH=['/opt/local/include/SDL'])
  env.Append(CPPPATH=['/opt/local/include'])
  env.Append(LIBPATH=['/opt/local/lib'])
  env.Append(LIBS=['SDL', 'SDL_image', 'SDLmain'])
  env.Append(CPPDEFINES=[('DN_SDL', 1)])

# GL

if use_gl:
  if is_mac:
    env.Append(LINKFLAGS=['-framework', 'OpenGL'])
  else:
    env.Append(LIBS=['GL'])
  env.Append(CPPDEFINES=[('DN_GL', 1)])

# Cuda

if use_cuda:
  env.Append(CPPDEFINES=[('DN_CUDA', 1)])
  env.Append(CPPPATH=[CUDA_PATH + '/include'])
  env.Append(LIBPATH=[CUDA_PATH + '/lib'])
  env.Append(LIBS=['cuda'])
  env.PrependENVPath('PATH', CUDA_PATH + '/bin')

# Sources

env.Append(CPPPATH=['.'])

src = []
src += glob.glob('src/*.cpp')
src.sort()

# Cuda
    
if not use_cuda:
  src.remove('src/cuda.cpp')
  src.remove('src/cudabvh.cpp')
else:
  for cu in glob.glob('src/*.cu'):
    b = os.path.basename(cu)
    b, ext = b.rsplit(".")

    if not env.Command(b + '.cubin', [cu, 'src/cudavec.h'], \
      'nvcc -m32 -maxrregcount %d -use_fast_math -arch sm_11 -cubin %s' % (cuda_regcount, cu)):
      os.exit(1)

# Done!

env.Program('gpurt', src)
