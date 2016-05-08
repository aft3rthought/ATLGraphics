

#pragma once

#include "ATF/ATLGLResource.h"
#include "ATLUtil/math2d.h"

#include <vector>
#include <string>

class NomBitmapFontChar
{
public:
    char m_char;
    float m_advance;
    atl::point2f m_offset;
    atl::size2f m_size;
    atl::box2f m_texBounds;
    
    NomBitmapFontChar(char in_char,
                      float in_advance,
                      const atl::point2f & in_offset,
                      const atl::size2f & in_size,
                      const atl::box2f & in_texBounds);
};

class NomBitmapFont
{
public:
    std::string m_fontName;
    float m_spaceCharacterSize;
    ATLGLTexture m_texture;
    std::vector<NomBitmapFontChar> m_fontChars;
    atl::point2f m_pixTexelStride;
    int32_t m_fontSize;
    float m_boundsYMin;
    float m_boundsYMax;
    float m_lineHeight;
    
    NomBitmapFont(const std::string & in_fontName,
                  int32_t in_fontSize,
                  float in_spaceCharacterSize,
                  const atl::point2f & in_pixTexelStride,
                  float in_boundsYMin,
                  float in_boundsYMax,
                  float m_lineHeight);
    
    ~NomBitmapFont();
};

class NomBitmapFontResource
{
public:
    void loadFromFile(const std::string & inFileName);
    bool ready() const { return pmFont != nullptr; }
    const NomBitmapFont & font() const { return *pmFont; }
    
    ~NomBitmapFontResource();
    
private:
    NomBitmapFont* pmFont;
};
