

#include "ATLGLResource.h"
#include "ATLUtil/debug_break.h"

namespace atl_graphics_namespace_config
{
    base_resource::~base_resource()
    {
        SGDebugBreakIf(valid(), "GL resource not set to invalid before owner is destroyed! Is a resource being lost?");
    }

#ifdef DEBUG
    base_resource::operator GLuint () const
    {
        SGDebugBreakIf(isInvalid(), "Accesing invalid GL resource");
        return pm_resource;
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
        SGDebugBreakIf(!glIsTexture(internal_gl_handle), "Resource isn't valid when calling free, is everything OK?");
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
        SGDebugBreakIf(!glIsProgram(internal_gl_handle), "Resource isn't valid when calling free, is everything OK?");
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
        SGDebugBreakIf(!glIsBuffer(internal_gl_handle), "Resource isn't valid when calling free, is everything OK?");
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
        glGenVertexArraysOES(1, &pm_resource);
        check_gl_errors();
        return pm_resource != 0;
#endif

#ifdef PLATFORM_OSX
        glGenVertexArraysAPPLE(1, &pm_resource);
        :check_gl_errors();
        return pm_resource != 0;
#endif
    }

    bool vertex_array_resource::bind() const
    {
#ifdef PLATFORM_WINDOWS
        return false;
#endif

#ifdef PLATFORM_IOS
#ifdef DEBUG
        SGDebugBreakIf(!glIsVertexArrayOES(pm_resource), "Resource isn't valid when calling bind, is everything OK?");
#endif
        if(pm_resource != 0)
        {
            glBindVertexArrayOES(pm_resource);
            atf::check_gl_errors();
            return true;
        }
        return false;
#endif

#ifdef PLATFORM_OSX
#ifdef DEBUG
        SGDebugBreakIf(!glIsVertexArrayAPPLE(pm_resource), "Resource isn't valid when calling bind, is everything OK?");
#endif
        if(pm_resource != 0)
        {
            glBindVertexArrayAPPLE(pm_resource);
            atf::check_gl_errors();
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
        atf::check_gl_errors();
        return true;
#endif

#ifdef PLATFORM_OSX
        glBindVertexArrayAPPLE(0);
        atf::check_gl_errors();
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
        SGDebugBreakIf(!glIsVertexArrayOES(pm_resource), "Resource isn't valid when calling free, is everything OK?");
#endif
        if(pm_resource != 0)
        {
            glDeleteVertexArraysOES(1, &pm_resource);
            atf::check_gl_errors();
            pm_resource = 0;
            return true;
        }
        return false;
#endif

#ifdef PLATFORM_OSX
#ifdef DEBUG
        SGDebugBreakIf(!glIsVertexArrayAPPLE(pm_resource), "Resource isn't valid when calling free, is everything OK?");
#endif
        if(pm_resource != 0)
        {
            glDeleteVertexArraysAPPLE(1, &pm_resource);
            atf::check_gl_errors();
            pm_resource = 0;
            return true;
        }
        return false;
#endif
    }
}
