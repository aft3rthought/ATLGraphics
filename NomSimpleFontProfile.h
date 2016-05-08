

#pragma once

#include <string>
#include "ATLUtil/color.h"

class NomSimpleFontProfile
{
    const static atl::color pcsm_defaultColor;
    
public:
    float m_edges[4];
    atl::color m_colors[4];
    
    NomSimpleFontProfile(float in_edge1,
                         const atl::color & in_color1,
                         float in_edge2 = 1.f,
                         const atl::color & in_color2 = pcsm_defaultColor,
                         float in_edge3 = 1.f,
                         const atl::color & in_color3 = pcsm_defaultColor,
                         float in_edge4 = 1.f,
                         const atl::color & in_color4 = pcsm_defaultColor);
    
    void colorLerp(atl::color in_color);
    void colorMult(atl::color in_color);
};
