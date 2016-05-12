
#pragma once

#include "ATLUtil/math2d.h"
#include "atl_graphics_namespace.h"

namespace atl_graphics_namespace_config
{
    struct stage;

    struct shared_renderer_state
    {
    private:
        void * pm_currentRenderer;

    public:
        shared_renderer_state(const atl::stage & in_stage, int in_width, int in_height);

        atl::box2f m_currentBounds;
        int m_glViewportWidth;
        int m_glViewportHeight;

        bool setAsCurrentRenderer(void * in_object)
        {
            if(in_object != pm_currentRenderer)
            {
                pm_currentRenderer = in_object;
                return true;
            }
            return false;
        }
    };
}
