

#pragma once

#include "atl_graphics_namespace.h"
#include "ATLUtil/debug_break.h"

#ifdef PLATFORM_WINDOWS
// OpenGL ES includes
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// EGL includes
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#define GL_RED_NOM GL_RED_EXT
#endif

#ifdef PLATFORM_IOS
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <GLKit/GLKMath.h>

#define GL_RED_NOM GL_RED_EXT
#endif

#ifdef PLATFORM_OSX
#include <OpenGL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define GL_RED_NOM GL_RED
#endif

namespace atl_graphics_namespace_config
{
    inline bool check_gl_errors()
    {
        GLenum atl_gl_err_define_local = GL_NO_ERROR;
        while((atl_gl_err_define_local = glGetError()) != GL_NO_ERROR)
        {
            atl_log_function_impl("%s, line %d: gl error detected: %x\n", __FILE__, __LINE__, atl_gl_err_define_local);
            atl::print_callstack();
        }
        return atl_gl_err_define_local != GL_NO_ERROR;
    }
}
