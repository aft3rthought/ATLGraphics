

#pragma once

#include "ATLUtil/math2d.h"

namespace atl {
    class stage;
}

class NomRendererState
{
private:
    void * pm_currentRenderer;
    
public:
    NomRendererState(const atl::stage & in_stage, int in_width, int in_height);
    
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