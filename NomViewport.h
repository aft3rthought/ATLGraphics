

#pragma once

#include "ATLUtil/math2d.h"

namespace atl {
    class stage;
}
class NomRendererState;

class NomViewport
{
private:
    int pm_oldViewport[4];
    atl::box2f pm_oldScreenBounds;
    NomRendererState & pm_rendererState;
    
public:
    NomViewport(const atl::box2f & l_trimArea,
                const atl::stage & in_stage,
                NomRendererState & in_rendererState);
    ~NomViewport();
};