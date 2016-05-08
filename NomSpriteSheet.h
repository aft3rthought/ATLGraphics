

#pragma once

#include <vector>
#include <string>
#include "ATF/NomSpriteFrame.h"

struct NomGLTextureCollection
{
    std::vector<unsigned int> m_textures;
    ~NomGLTextureCollection();
};

namespace atf {
    enum class TextureFilteringMode {
        Nearest,
        Linear,
    };
}

namespace NomSpriteSheet
{
    void parseSpriteSheet(const std::string & inFilePath,
                          atf::TextureFilteringMode inFilteringMode,
                          std::vector<NomSpriteFrame> & outSprites,
                          NomGLTextureCollection & outTextures);
}
