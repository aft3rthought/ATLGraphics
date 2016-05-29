

#include "NomViewport.h"

#include "ATLCrossPlatform/stage.h"
#include "ATF/NomRendererState.h"
#include "ATF/NomRenderingHeaders.h"

namespace atl_graphics_namespace_config
{
    viewport::viewport(const atl::box2f & in_trim_area,
                       const atl::stage & in_stage,
                       shared_renderer_state & in_renderer_state)
        :
        renderer_state(in_renderer_state)
    {
        old_screen_bounds = in_renderer_state.m_currentBounds;

        glGetIntegerv(GL_VIEWPORT, old_viewport);

        float l_contentLPct = (in_trim_area.l - in_stage.bounds().l) / in_stage.dimensions().w;
        float l_contentBPct = (in_trim_area.b - in_stage.bounds().b) / in_stage.dimensions().h;
        float l_contentWPct = in_trim_area.width() / in_stage.dimensions().w;
        float l_contentHPct = in_trim_area.height() / in_stage.dimensions().h;

        glViewport((GLint)(l_contentLPct * in_renderer_state.m_glViewportWidth),
                   (GLint)(l_contentBPct * in_renderer_state.m_glViewportHeight),
                   (GLint)(l_contentWPct * in_renderer_state.m_glViewportWidth),
                   (GLint)(l_contentHPct * in_renderer_state.m_glViewportHeight));

        in_renderer_state.m_currentBounds = in_trim_area;
    }

    viewport::~viewport()
    {
        renderer_state.m_currentBounds = old_screen_bounds;

        glViewport(old_viewport[0],
                   old_viewport[1],
                   old_viewport[2],
                   old_viewport[3]);
    }
}
