

#pragma once

#include "ATLUtil/math2d.h"
#include "atl_graphics_namespace.h"
#include "ATF/NomRenderingHeaders.h"
#include "ATF/texture_filtering_mode.h"

namespace atl_graphics_namespace_config
{
    enum class framebuffer_color_mode : int {
        single_channel,
        rgb,
        rgba
    };
    
    struct shared_renderer_state;
    
    struct framebuffer_descriptor
    {
        GLsizei width;
        GLsizei height;
        atl::texture_filtering_mode filtering_mode;
        atl::framebuffer_color_mode color_mode;
        
        GLuint framebuffer_name;
        GLuint texture_name;
        bool valid;
    };
    
    void make_framebuffer(framebuffer_descriptor & descriptor);
    
    struct framebuffer_scope
    {
        int original_viewport[4];
        atl::box2f original_screen_bounds;
        shared_renderer_state & renderer_state;
        
        framebuffer_scope(const framebuffer_descriptor & descriptor, shared_renderer_state & in_renderer_state);
        ~framebuffer_scope();
    };
}
