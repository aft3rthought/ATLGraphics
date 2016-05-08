

#pragma once

#include "ATLUtil/math2d.h"
#include <string>

class NomSpriteFrame
{
public:
    std::string m_fileName;
    unsigned int m_textureSheetGlHandle;    // Texture sheet GL handle
    atl::box2f m_imgTexCoords;             // Tex coordinates in texture sheet
    atl::box2f m_area;                     // Area the image will be drawn in, relative to the draw pos, when untranslated by translation, rotation, or scale
    
    NomSpriteFrame();
    
    void set(unsigned int in_glHandle,
             const atl::box2f & in_texCoords,
             const atl::box2f & in_area);
};