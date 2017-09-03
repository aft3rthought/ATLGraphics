

#pragma once

#include <string>
#include "ATLUtil/color.h"
#include "atl_graphics_namespace.h"

namespace atl_graphics_namespace_config
{
    struct simple_font_profile
    {
        const static atl::color pcsm_defaultColor;

    public:
        float df_radius[4];
        bool scale_radius_to_view[4];
        float m_edges[4];
        atl::color m_colors[4];

        simple_font_profile();

        void colorLerp(atl::color in_color);
        void colorMult(atl::color in_color);
    };
}
