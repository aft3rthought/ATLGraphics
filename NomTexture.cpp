

#include "./NomTexture.h"

#include "ATF/lodepng.h"
#include "ATF/NomImageLoading.h"

void NomTexture::loadFromFile(const std::string & inFileName, FilteringMode inFilteringMode)
{
    if(pmHandle.isInvalid())
    {
        unsigned char* lImageData;
        unsigned int lWidth, lHeight;
        
        auto lError = lodepng_decode32_file(&lImageData, &lWidth, &lHeight, inFileName.c_str());
        if(lError == 0)
        {
            //
            pmHandle.alloc();
            
            glBindTexture(GL_TEXTURE_2D, pmHandle);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RGBA,
                         lWidth,
                         lHeight,
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         lImageData);
            
            auto lGLMode = inFilteringMode == FilteringMode::Linear ? GL_LINEAR : GL_NEAREST;
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, lGLMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, lGLMode);
        }
    }
    else
    {
        SGDebugBreak("Handle re-loading NomTexture?");
    }
}