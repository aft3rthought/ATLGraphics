
#include "./NomRendererState.h"

#include "ATLCrossPlatform/stage.h"
#include "ATF/NomRenderingHeaders.h"

namespace atl_graphics_namespace_config
{
    shared_renderer_state::shared_renderer_state(const atl::stage & in_stage, int in_width, int in_height)
    {
        m_currentBounds = in_stage.bounds();
        m_glViewportWidth = in_width;
        m_glViewportHeight = in_height;
    }
}
