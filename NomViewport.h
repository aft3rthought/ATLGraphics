

#pragma once

#include "ATLUtil/math2d.h"
#include "atl_graphics_namespace.h"

namespace atl_graphics_namespace_config
{
    struct stage;
    struct shared_renderer_state;

    class viewport
    {
    private:
        int old_viewport[4];
        atl::box2f old_screen_bounds;
        shared_renderer_state & renderer_state;

    public:
        viewport(shared_renderer_state & in_renderer_state);
        viewport(const atl::box2f & in_trim_area,
                 const atl::stage & in_stage,
                 shared_renderer_state & in_renderer_state);
        ~viewport();
        void acquire(const atl::box2f & in_trim_area,
                     const atl::stage & in_stage);
    };
}
