

#pragma once

#include "atl_graphics_namespace.h"

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
        bool retVal = true;
        GLenum atl_gl_err_define_local = GL_NO_ERROR;
        while((atl_gl_err_define_local = glGetError()) != GL_NO_ERROR)
        {
#ifdef DEBUG
            printf("atl::graphics - GL ERROR DETECTED: %i\n", atl_gl_err_define_local);
#endif
            retVal = false;
        }
        return retVal;
    }
}