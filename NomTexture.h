

#pragma once

#include "ATF/ATLGLResource.h"
#include <string>

class NomTexture
{
private:
    ATLGLTexture pmHandle;

public:
    enum class FilteringMode
    {
        Linear,
        Nearest
    };
    
    void loadFromFile(const std::string & inFileName, FilteringMode inFilteringMode);
    bool read() const { return pmHandle.isValid(); }
    ATLGLTexture handle() const { return pmHandle; }
};