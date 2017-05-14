

#include "NomSpriteSheet.h"
#include "ATLUtil/numeric_util.h"
#include "ATLUtil/bit_string.h"
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
                    auto file_data = internal_loaded_file_data.data();
                    atl::input_bit_string_type bit_string(file_data.data(), file_data.data() + file_data.size());
                    
                    // Load frames:
                    glActiveTexture(GL_TEXTURE0);

                    uint32_t numSheets;
                    atl::bit_string_read_chunked_integer<uint32_t>(bit_string, numSheets, 3);
                    uint32_t totalSprites;
                    atl::bit_string_read_chunked_integer<uint32_t>(bit_string, totalSprites, 7);

                    sprites.resize(totalSprites);
                    textures.resize(numSheets);

                    for(auto & sheetId : textures)
                    {
                        // give the sheet a valid opengl texture ID
                        glGenTextures(1, &sheetId);
                        check_gl_errors();

                        uint32_t fontSheetWidthPower, fontSheetHeightPower;
                        atl::bit_string_read_ranged_integer<uint32_t>(bit_string, fontSheetWidthPower, 0, 12);
                        atl::bit_string_read_ranged_integer<uint32_t>(bit_string, fontSheetHeightPower, 0, 12);
                        uint32_t sheetWidth = 1 << fontSheetWidthPower;
                        uint32_t sheetHeight = 1 << fontSheetHeightPower;

                        atl::size2f sheetSize(sheetWidth, sheetHeight);

                        uint32_t sheetSprites;
                        atl::bit_string_read_ranged_integer<uint32_t>(bit_string, sheetSprites, 0, totalSprites);
                        while(sheetSprites-- > 0)
                        {
                            // Deserialize the frame index:
                            uint32_t spriteIndex;
                            atl::bit_string_read_ranged_integer<uint32_t>(bit_string, spriteIndex, 0, totalSprites - 1);

                            // Deserialize the texture coordinates:
                            atl::box2f l_texCoords;
                            {
                                uint32_t l_texT, l_texR, l_texB, l_texL;
                                atl::bit_string_read_ranged_integer<uint32_t>(bit_string, l_texT, 0, sheetHeight);
                                atl::bit_string_read_ranged_integer<uint32_t>(bit_string, l_texR, 0, sheetWidth);
                                atl::bit_string_read_ranged_integer<uint32_t>(bit_string, l_texB, 0, sheetHeight);
                                atl::bit_string_read_ranged_integer<uint32_t>(bit_string, l_texL, 0, sheetWidth);

                                if(l_texR - l_texL == 1)
                                {
                                    l_texCoords.l = l_texCoords.r = (float(l_texL) + 0.5f) / float(sheetWidth);
                                }
                                else
                                {
                                    l_texCoords.l = (float(l_texL)) / float(sheetWidth);
                                    l_texCoords.r = (float(l_texR)) / float(sheetWidth);
                                }

                                if(l_texB - l_texT == 1)
                                {
                                    l_texCoords.t = l_texCoords.b = (float(l_texT) + 0.5f) / float(sheetHeight);
                                }
                                else
                                {
                                    l_texCoords.t = (float(l_texT)) / float(sheetHeight);
                                    l_texCoords.b = (float(l_texB)) / float(sheetHeight);
                                }
                            }

                            // Deserialize the drawn area:
                            atl::box2f l_area;
                            atl::bit_string_read_values(bit_string, l_area.t, l_area.r, l_area.b, l_area.l);

                            sprites[spriteIndex].set(sheetId, l_texCoords, l_area);
                        }

                        // Decode PNG data and upload to card:
                        glBindTexture(GL_TEXTURE_2D, sheetId);

                        int32_t numPNGBytesInt32;
                        atl::bit_string_read_value(bit_string, numPNGBytesInt32);
                        size_t numPNGBytes = numPNGBytesInt32;
                        atl::bit_string_skip_to_next_byte(bit_string);
                        const unsigned char * l_pngBytes = bit_string.ptr;
                        bit_string.ptr += numPNGBytes;

                        unsigned char * imageDataOut = nullptr;
                        unsigned int imageWidthOut;
                        unsigned int imageHeightOut;
                        unsigned int decodeError = lodepng_decode32(&imageDataOut, &imageWidthOut, &imageHeightOut, l_pngBytes, numPNGBytes);
                        atl_fatal_if(decodeError > 0, "Couldn't decode PNG in sprite sheet");
                        atl_fatal_if(imageWidthOut != sheetWidth || imageHeightOut != sheetHeight, "PNG in sprite sheet of unexpected size");

                        // Allocate GL resource:
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                        check_gl_errors();
                        glTexImage2D(GL_TEXTURE_2D,
                                        0,
                                        GL_RGBA,
                                        atl::integer_cast<GLsizei>(sheetWidth, [](){}),
                                        atl::integer_cast<GLsizei>(sheetHeight, [](){}),
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
