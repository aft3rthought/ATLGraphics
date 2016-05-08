

#pragma once

#ifdef PLATFORM_IOS
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <GLKit/GLKMath.h>

#define GL_RED_NOM GL_RED_EXT
#define glGenVertexArrays_NOM glGenVertexArraysOES
#define glBindVertexArray_NOM glBindVertexArrayOES
#define glDeleteVertexArrays_NOM glDeleteVertexArraysOES
#define glIsVertexArray_NOM glIsVertexArrayOES

#endif

#ifdef PLATFORM_OSX
#include <OpenGL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define GL_RED_NOM GL_RED
#define glGenVertexArrays_NOM glGenVertexArraysAPPLE
#define glBindVertexArray_NOM glBindVertexArrayAPPLE
#define glDeleteVertexArrays_NOM glDeleteVertexArraysAPPLE
#define glIsVertexArray_NOM glIsVertexArrayAPPLE

#endif

namespace atf {
    inline bool check_gl_errors()
    {
        bool retVal = true;
        #ifdef DEBUG
        GLenum atl_gl_err_define_local = GL_NO_ERROR;
        while((atl_gl_err_define_local = glGetError()) != GL_NO_ERROR)
        {
            printf("ATF - GL ERROR DETECTED: %i\n", atl_gl_err_define_local);
            retVal = false;
        }
        #endif
        return retVal;
    }
}