

#pragma once

#include "ATF/NomRenderingHeaders.h"

namespace atl_graphics_namespace_config
{
    class base_resource
    {
    protected:
        GLuint internal_gl_handle;

    public:
        base_resource() : internal_gl_handle(0) {};
        ~base_resource();

#ifdef DEBUG
        operator GLuint () const;
#else
        operator GLuint () const { return internal_gl_handle; }
#endif
        bool valid() const { return internal_gl_handle != 0; }
    };

    class texture_resource : public base_resource
    {
    public:
        void alloc();
        void free();
    };

    class shader_program_resource : public base_resource
    {
    public:
        void alloc();
        void free();
    };

    class buffer_resource : public base_resource
    {
    public:
        void alloc();
        void free();
    };

    class vertex_array_resource : public base_resource
    {
    public:
        bool alloc();
        bool bind() const;
        bool unbind() const;
        bool free();
    };
}