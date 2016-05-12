

#include "NomSimpleFontProfile.h"

namespace atl_graphics_namespace_config
{
    const atl::color simple_font_profile::pcsm_defaultColor = atl::color(0.f, 0.f, 0.f, 0.f);

    simple_font_profile::simple_font_profile(float in_edge1,
                                               const atl::color & in_color1,
                                               float in_edge2,
                                               const atl::color & in_color2,
                                               float in_edge3,
                                               const atl::color & in_color3,
                                               float in_edge4,
                                               const atl::color & in_color4) :
        m_edges{in_edge1, in_edge2, in_edge3, in_edge4},
        m_colors{in_color1, in_color2, in_color3, in_color4}
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