

#include "ATLGLResource.h"
#include "ATLUtil/debug_break.h"

namespace atl_graphics_namespace_config
{
    base_resource::~base_resource()
    {
        atl_fatal_if(valid(), "GL resource not set to invalid before owner is destroyed! Is a resource being lost?");
    }

#ifdef DEBUG
    base_resource::operator GLuint () const
    {
        atl_fatal_assert(valid(), "Accesing invalid GL resource");
        return internal_gl_handle;
    }
#endif

    void texture_resource::alloc()
    {
        glGenTextures(1, &internal_gl_handle);
        check_gl_errors();
    }

    void texture_resource::free()
    {
#ifdef DEBUG
        atl_fatal_assert(glIsTexture(internal_gl_handle), "Resource isn't valid when calling free, is everything OK?");
#endif
        glDeleteTextures(1, &internal_gl_handle);
        check_gl_errors();
        internal_gl_handle = 0;
    }

    void shader_program_resource::alloc()
    {
        internal_gl_handle = glCreateProgram();
        check_gl_errors();
    }

    void shader_program_resource::free()
    {
#ifdef DEBUG
        atl_fatal_assert(glIsProgram(internal_gl_handle), "Resource isn't valid when calling free, is everything OK?");
#endif
        glDeleteProgram(internal_gl_handle);
        check_gl_errors();
        internal_gl_handle = 0;
    }

    void buffer_resource::alloc()
    {
        glGenBuffers(1, &internal_gl_handle);
        check_gl_errors();
    }

    void buffer_resource::free()
    {
#ifdef DEBUG
        atl_fatal_assert(glIsBuffer(internal_gl_handle), "Resource isn't valid when calling free, is everything OK?");
#endif
        glDeleteBuffers(1, &internal_gl_handle);
        check_gl_errors();
        internal_gl_handle = 0;
    }

    bool vertex_array_resource::alloc()
    {
#ifdef PLATFORM_WINDOWS
        return false;
#endif

#ifdef PLATFORM_IOS
        glGenVertexArraysOES(1, &internal_gl_handle);
        check_gl_errors();
        return internal_gl_handle != 0;
#endif

#ifdef PLATFORM_OSX
        glGenVertexArraysAPPLE(1, &internal_gl_handle);
        check_gl_errors();
        return internal_gl_handle != 0;
#endif
    }

    bool vertex_array_resource::bind() const
    {
#ifdef PLATFORM_WINDOWS
        return false;
#endif

#ifdef PLATFORM_IOS
        if(internal_gl_handle != 0)
        {
            glBindVertexArrayOES(internal_gl_handle);
            check_gl_errors();
            return true;
        }
        return false;
#endif

#ifdef PLATFORM_OSX
        if(internal_gl_handle != 0)
        {
            glBindVertexArrayAPPLE(internal_gl_handle);
            check_gl_errors();
            return true;
        }
        return false;
#endif
    }

    bool vertex_array_resource::unbind() const
    {
#ifdef PLATFORM_WINDOWS
        return false;
#endif

#ifdef PLATFORM_IOS
        glBindVertexArrayOES(0);
        check_gl_errors();
        return true;
#endif

#ifdef PLATFORM_OSX
        glBindVertexArrayAPPLE(0);
        check_gl_errors();
        return true;
#endif
    }

    bool vertex_array_resource::free()
    {
#ifdef PLATFORM_WINDOWS
        return false;
#endif

#ifdef PLATFORM_IOS
#ifdef DEBUG
        atl_fatal_assert(glIsVertexArrayOES(internal_gl_handle), "Resource isn't valid when calling free, is everything OK?");
#endif
        if(internal_gl_handle != 0)
        {
            glDeleteVertexArraysOES(1, &internal_gl_handle);
            check_gl_errors();
            internal_gl_handle = 0;
            return true;
        }
        return false;
#endif

#ifdef PLATFORM_OSX
#ifdef DEBUG
        atl_fatal_assert(glIsVertexArrayAPPLE(internal_gl_handle), "Resource isn't valid when calling free, is everything OK?");
#endif
        if(internal_gl_handle != 0)
        {
            glDeleteVertexArraysAPPLE(1, &internal_gl_handle);
            check_gl_errors();
            internal_gl_handle = 0;
            return true;
        }
        return false;
#endif
    }
}
