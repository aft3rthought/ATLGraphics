//
//  NomRendererState.cpp
//  NomNomCats
//
//  Created by Maxwell Robinson on 11/15/13.
//  Copyright (c) 2013 All The Loot. All rights reserved.
//

#include "./NomRendererState.h"

#include "ATLCrossPlatform/stage.h"
#include "ATF/NomRenderingHeaders.h"

NomRendererState::NomRendererState(const atl::stage & in_stage, int in_width, int in_height)
{
    m_currentBounds = in_stage.bounds();
    m_glViewportWidth = in_width;
    m_glViewportHeight = in_height;
}