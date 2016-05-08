

#include "./NomViewport.h"

#include "ATLCrossPlatform/stage.h"
#include "ATF/NomRendererState.h"
#include "ATF/NomRenderingHeaders.h"

NomViewport::NomViewport(const atl::box2f & l_trimArea,
                         const atl::stage & in_stage,
                         NomRendererState & in_rendererState) :
pm_rendererState(in_rendererState)
{
    pm_oldScreenBounds = in_rendererState.m_currentBounds;
    
    glGetIntegerv(GL_VIEWPORT, pm_oldViewport);

    float l_contentLPct = (l_trimArea.l - in_stage.bounds().l) / in_stage.dimensions().w;
    float l_contentBPct = (l_trimArea.b - in_stage.bounds().b) / in_stage.dimensions().h;
    float l_contentWPct = l_trimArea.width() / in_stage.dimensions().w;
    float l_contentHPct = l_trimArea.height() / in_stage.dimensions().h;

    glViewport((GLint)(l_contentLPct * in_rendererState.m_glViewportWidth),
               (GLint)(l_contentBPct * in_rendererState.m_glViewportHeight),
               (GLint)(l_contentWPct * in_rendererState.m_glViewportWidth),
               (GLint)(l_contentHPct * in_rendererState.m_glViewportHeight));

    in_rendererState.m_currentBounds = l_trimArea;
}

NomViewport::~NomViewport()
{
    pm_rendererState.m_currentBounds = pm_oldScreenBounds;
    
    glViewport(pm_oldViewport[0],
               pm_oldViewport[1],
               pm_oldViewport[2],
               pm_oldViewport[3]);
}