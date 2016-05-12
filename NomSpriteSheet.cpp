

#include "NomSpriteSheet.h"
#include "ATLUtil/serialize.h"
#include "ATLUtil/debug_break.h"
#include "lodepng.h"
#include "ATF/NomRenderingHeaders.h"
#include "ATF/NomFiles.h"

namespace atl_graphics_namespace_config
{
    sprite_sheet::~sprite_sheet()
    {
        for(auto & l_resouce : textures)
            glDeleteTextures(1, &l_resouce);
    }

    sprite_sheet::sprite_sheet(const char * in_sheet_file_path, const texture_filtering_mode in_filtering_mode)
    :
    internal_status(sprite_sheet_status::waiting_to_load),
    internal_texture_filtering_mode(in_filtering_mode),
    internal_sheet_file_path(in_sheet_file_path)
    {}

    void sprite_sheet::incremental_load(const application_folder & in_application_folder)
    {
        switch(internal_status)
        {
            case sprite_sheet_status::ready:
            case sprite_sheet_status::failed:
                break;
            case sprite_sheet_status::waiting_to_load:
            {
                if(internal_loaded_file_data.status() == file_data_status::waiting_to_load)
                {
                    internal_status = sprite_sheet_status::loading;
                    internal_loaded_file_data.load(in_application_folder, internal_sheet_file_path.c_str());
                }
                break;
            }
            case sprite_sheet_status::loading:
            {
                if(internal_loaded_file_data.status() == file_data_status::loading)
                {
                }
                else if(internal_loaded_file_data.status() == file_data_status::ready)
                {
                    bits_in l_stream;
                    l_stream.attach_bytes(internal_loaded_file_data.data().data(), (int)internal_loaded_file_data.data().size());

                    // Load frames:
                    glActiveTexture(GL_TEXTURE0);

                    int32_t numSheets = l_stream.read_uint32_var_bit(3);
                    int32_t totalSprites = l_stream.read_uint32_var_bit(7);

                    sprites.resize(totalSprites);
                    textures.resize(numSheets);

                    for(auto & sheetId : textures)
                    {
                        int32_t sheetWidth = 1 << l_stream.read_ranged_int(0, 12);
                        int32_t sheetHeight = 1 << l_stream.read_ranged_int(0, 12);

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

                            sprites[spriteIndex].set(sheetId, l_texCoords, l_area);
                        }

                        // Decode PNG data and upload to card:
                        glBindTexture(GL_TEXTURE_2D, sheetId);

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

                        // Allocate GL resource:
                        glGenTextures(1, &sheetId);
                        check_gl_errors();
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                        check_gl_errors();
                        glTexImage2D(GL_TEXTURE_2D,
                                        0,
                                        GL_RGBA,
                                        sheetWidth,
                                        sheetHeight,
                                        0,
                                        GL_RGBA,
                                        GL_UNSIGNED_BYTE,
                                        imageDataOut);
                        check_gl_errors();
                        GLint lFilteringMode = internal_texture_filtering_mode == texture_filtering_mode::linear ? GL_LINEAR : GL_NEAREST;
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, lFilteringMode);
                        check_gl_errors();
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, lFilteringMode);
                        check_gl_errors();

                        delete[] imageDataOut;
                    }
                    internal_loaded_file_data.free();
                    internal_status = sprite_sheet_status::ready;
                }
                else
                {
                    internal_status = sprite_sheet_status::failed;
                }
                break;
            }
        }
    }
}
