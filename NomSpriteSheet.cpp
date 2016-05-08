

#include "NomSpriteSheet.h"
#include "ATF/NomRenderingHeaders.h"
#include "ATLUtil/serialize.h"
#include "ATLUtil/debug_break.h"
#include "lodepng.h"

NomGLTextureCollection::~NomGLTextureCollection()
{
    for(auto & l_resouce : m_textures)
        glDeleteTextures(1, &l_resouce);
}

void NomSpriteSheet::parseSpriteSheet(const std::string & inFilePath,
                                      atf::TextureFilteringMode inFilteringMode,
                                      std::vector<NomSpriteFrame> & outSprites,
                                      NomGLTextureCollection & outTextures)
{
    // Initialize stream:
    std::ifstream l_file(inFilePath.c_str(), std::ios::in);
    SGFatalErrorIf(!l_file.good(), "Missing sprite sheet");
    
    std::vector<unsigned char> l_sheetData;
    
    //
    std::streamoff l_size = 0;
    if(l_file.seekg(0, std::ios::end).good())
        l_size = l_file.tellg();
    
    if(l_file.seekg(0, std::ios::beg).good())
        l_size -= l_file.tellg();
    
    SGFatalErrorIf(l_size <= 0, "Sprite sheet has no data");
    
    size_t fileSize = (size_t)l_size;
    
    l_sheetData.resize(fileSize);
    l_file.read((char *)l_sheetData.data(), fileSize);
    
    atl::bits_in l_stream;
    l_stream.attach_bytes(l_sheetData.data(), (int)l_sheetData.size());
    
    // Load frames:
    glActiveTexture(GL_TEXTURE0);
    
    int32_t numSheets = l_stream.read_uint32_var_bit(3);
    int32_t totalSprites = l_stream.read_uint32_var_bit(7);
    
    outSprites.resize(totalSprites);
    outTextures.m_textures.resize(numSheets);
    
    for(auto & sheetId : outTextures.m_textures)
    {
        int32_t sheetWidth = 1 << l_stream.read_ranged_int(0, 12);
        int32_t sheetHeight = 1 << l_stream.read_ranged_int(0, 12);
        
        // Allocate GL resource:
        glGenTextures(1, &sheetId);
        SGFatalErrorIf(glGetError() != 0, "Failed to load texture");
        
        atl::size2f sheetSize(sheetWidth, sheetHeight);
        
        int32_t sheetSprites = l_stream.read_ranged_int(0, totalSprites);
        while(sheetSprites-- > 0)
        {
            // Deserialize the frame index:
            int32_t spriteIndex = l_stream.read_ranged_int(0, totalSprites - 1);
            
            // Deserialize the texture coordinates:
            atl::box2f l_texCoords;
            {
                int32_t l_texT = l_stream.read_ranged_int(0, sheetHeight);
                int32_t l_texR = l_stream.read_ranged_int(0, sheetWidth);
                int32_t l_texB = l_stream.read_ranged_int(0, sheetHeight);
                int32_t l_texL = l_stream.read_ranged_int(0, sheetWidth);
                
                if(l_texR - l_texL == 1)
                {
                    l_texCoords.l =
                    l_texCoords.r = (float(l_texL) + 0.5f) / float(sheetWidth);
                }
                else
                {
                    l_texCoords.l = (float(l_texL)) / float(sheetWidth);
                    l_texCoords.r = (float(l_texR)) / float(sheetWidth);
                }
                
                if(l_texB - l_texT == 1)
                {
                    l_texCoords.t =
                    l_texCoords.b = (float(l_texT) + 0.5f) / float(sheetHeight);
                }
                else
                {
                    l_texCoords.t = (float(l_texT)) / float(sheetHeight);
                    l_texCoords.b = (float(l_texB)) / float(sheetHeight);
                }
            }
            
            // Deserialize the drawn area:
            atl::box2f l_area(l_stream.read_float(),
                                 l_stream.read_float(),
                                 l_stream.read_float(),
                                 l_stream.read_float());
            
            outSprites[spriteIndex].set(sheetId, l_texCoords, l_area);
        }
        
        // Decode PNG data and upload to card:
        glBindTexture(GL_TEXTURE_2D, sheetId);
        
        {
            int32_t numPNGBytes = l_stream.read_int32();
            l_stream.skip_to_next_byte();
            const unsigned char * l_pngBytes = l_stream.current_data_pointer();
            l_stream.advance_data_pointer(numPNGBytes);
            
            unsigned char * imageDataOut = nullptr;
            unsigned int imageWidthOut;
            unsigned int imageHeightOut;
            unsigned int decodeError = lodepng_decode32(&imageDataOut, &imageWidthOut, &imageHeightOut, l_pngBytes, numPNGBytes);
            SGFatalErrorIf(decodeError > 0, "Couldn't decode PNG in sprite sheet");
            SGFatalErrorIf(imageWidthOut != sheetWidth || imageHeightOut != sheetHeight, "PNG in sprite sheet of unexpected size");
            
            // Diag logging:
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RGBA,
                         sheetWidth,
                         sheetHeight,
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         imageDataOut);
            GLint lFilteringMode = inFilteringMode == atf::TextureFilteringMode::Linear ? GL_LINEAR : GL_NEAREST;
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,lFilteringMode);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,lFilteringMode);
            
            delete [] imageDataOut;
        }
    }
}
