

#pragma once

#include "ATF/ATLGLResource.h"
#include "ATLUtil/math2d.h"

#include <vector>
#include <string>

class NomFontChar
{
public:
    char m_char;
    float m_advance;
    atl::point2f m_offset;
    atl::size2f m_size;
    atl::box2f m_texBounds;
    
    NomFontChar(char in_char,
                float in_advance,
                const atl::point2f & in_offset,
                const atl::size2f & in_size,
                const atl::box2f & in_texBounds);
};

class NomFont
{
public:
    std::string m_fontName;
    float m_spaceCharacterSize;
    ATLGLTexture m_texture;
    std::vector<NomFontChar> m_fontChars;
    atl::point2f m_pixTexelStride;
    float m_boundsYMin;
    float m_boundsYMax;
    float m_lineHeight;
    
    NomFont(const std::string & in_fontName,
            float in_spaceCharacterSize,
            const atl::point2f & in_pixTexelStride,
            float in_boundsYMin,
            float in_boundsYMax,
            float m_lineHeight);
    
    ~NomFont();
};

class NomFontResource
{
public:
    void loadFromFile(const std::string & inFileName);
    bool ready() const { return pmFont != nullptr; }
    const NomFont & font() const { return *pmFont; }
    
    ~NomFontResource();
    
private:
    NomFont* pmFont;
};
