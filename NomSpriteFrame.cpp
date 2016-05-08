

#include "./NomSpriteFrame.h"

NomSpriteFrame::NomSpriteFrame() :
    m_textureSheetGlHandle(0)
{}

void NomSpriteFrame::set(unsigned int in_glHandle,
                         const atl::box2f & in_texCoords,
                         const atl::box2f & in_area)
{
    m_textureSheetGlHandle = in_glHandle;
    m_area = in_area;
    m_imgTexCoords = in_texCoords;
}
