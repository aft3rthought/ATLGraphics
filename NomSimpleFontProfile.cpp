

#include "NomSimpleFontProfile.h"

namespace atl_graphics_namespace_config
{
    const atl::color simple_font_profile::pcsm_defaultColor = atl::color(0.f, 0.f, 0.f, 0.f);

    simple_font_profile::simple_font_profile() :
    m_edges{1.f, 1.f, 1.f, 1.f},
    m_colors{{1.f, 1.f, 1.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f, 1.f, 1.f}},
    df_radius{0.f, 0.f, 0.f, 0.f},
    scale_radius_to_view{false, false, false, false}
    {}

    void simple_font_profile::colorLerp(atl::color in_color)
    {
        for(auto & l_color : m_colors)
            l_color.interpolate(atl::color(in_color.r, in_color.g, in_color.b, 1.f), in_color.a);
    }

    void simple_font_profile::colorMult(atl::color in_color)
    {
        for(auto & l_color : m_colors)
        {
            l_color.r *= in_color.r;
            l_color.g *= in_color.g;
            l_color.b *= in_color.b;
            l_color.a *= in_color.a;
        }
    }
}
