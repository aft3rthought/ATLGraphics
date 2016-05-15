#include "./NomFont.h"

#include "ATLUtil/serialize.h"
#include "ATF/lodepng.h"
#include "ATF/NomImageLoading.h"

namespace atl_graphics_namespace_config
{
    scalable_font_char::scalable_font_char(char in_char,
                         float in_advance,
                         const atl::point2f & in_offset,
                         const atl::size2f & in_size,
                         const atl::box2f & in_texBounds)
    :
    m_char(in_char),
    m_advance(in_advance),
    m_offset(in_offset),
    m_size(in_size),
    m_texBounds(in_texBounds)
    {}

    scalable_font::scalable_font(const char * in_file_name)
    :
    internal_file_name(in_file_name),
    internal_status(scalable_font_status::waiting_to_load)
    {}

    scalable_font::~scalable_font()
    {
        if(internal_font_data.m_texture.valid())
            internal_font_data.m_texture.free();
    }

    void scalable_font::incremental_load(const application_folder & in_application_folder)
    {
        switch(internal_status.load())
        {
            case scalable_font_status::ready:
            case scalable_font_status::failed:
                break;
            case scalable_font_status::waiting_to_load:
            {
                if(internal_loaded_file_data.status() == file_data_status::waiting_to_load)
                {
                    internal_status = scalable_font_status::loading;
                    internal_loaded_file_data.load(in_application_folder, internal_file_name.c_str());
                }
                break;
            }
            case scalable_font_status::loading:
            {
                switch(internal_loaded_file_data.status())
                {
                    default:
                        internal_status = scalable_font_status::failed;
                        break;
                    case file_data_status::loading:
                        break;
                    case file_data_status::ready:
                    {
                        atl::bits_in l_deserializer(internal_loaded_file_data.data().data(), internal_loaded_file_data.data().size());

                        struct l_CharInfo
                        {
                            char m_char;
                            float m_advance;
                            atl::point2f m_offset;
                            atl::size2f m_size;
                            int m_sheetXPos;
                            int m_sheetYPos;
                            int m_sheetWidth;
                            int m_sheetHeight;
                        };

                        int32_t fontSheetWidth = 1 << l_deserializer.read_ranged_int(0, 12);
                        int32_t fontSheetHeight = 1 << l_deserializer.read_ranged_int(0, 12);

                        float l_spaceCharSize = l_deserializer.read_float();
                        uint32_t l_numCharacters = l_deserializer.read_uint32_var_bit(7);

                        std::vector<l_CharInfo> l_charInfos;
                        l_charInfos.reserve(l_numCharacters);

                        float l_minBoundsY = std::numeric_limits<float>::max();
                        float l_maxBoundsY = -std::numeric_limits<float>::max();
                        while(l_numCharacters-- > 0)
                        {
                            l_charInfos.emplace_back();
                            l_charInfos.back().m_char = l_deserializer.read_byte();
                            l_charInfos.back().m_advance = l_deserializer.read_float();
                            float l_offset_x = l_deserializer.read_float();
                            float l_offset_y = l_deserializer.read_float();
                            l_charInfos.back().m_offset = atl::point2f(l_offset_x, l_offset_y);
                            float l_size_x = l_deserializer.read_float();
                            float l_size_y = l_deserializer.read_float();
                            l_charInfos.back().m_size = atl::size2f(l_size_x, l_size_y);
                            l_charInfos.back().m_sheetXPos = l_deserializer.read_ranged_int(0, fontSheetWidth);
                            l_charInfos.back().m_sheetYPos = l_deserializer.read_ranged_int(0, fontSheetHeight);
                            l_charInfos.back().m_sheetWidth = l_deserializer.read_ranged_int(0, fontSheetWidth);
                            l_charInfos.back().m_sheetHeight = l_deserializer.read_ranged_int(0, fontSheetHeight);

                            l_minBoundsY = std::min(l_minBoundsY, l_charInfos.back().m_offset.y);
                            l_maxBoundsY = std::max(l_maxBoundsY, l_charInfos.back().m_offset.y + l_charInfos.back().m_size.h);

                            l_charInfos.back().m_offset.x -= 32.f / 256.f;
                            l_charInfos.back().m_offset.y -= 32.f / 256.f;
                            l_charInfos.back().m_size.w += 64.f / 256.f;
                            l_charInfos.back().m_size.h += 64.f / 256.f;
                        }

                        int32_t l_pngSize = l_deserializer.read_int32();
                        l_deserializer.skip_to_next_byte();
                        const unsigned char * l_pngData = (const unsigned char *)l_deserializer.current_data_pointer();
                        l_deserializer.advance_data_pointer(l_pngSize);

                        // Read png:
                        unsigned char * l_fontSheetData;
                        unsigned int l_fontSheetWidth, l_fontSheetHeight;
                        auto error = lodepng_decode_memory(&l_fontSheetData,
                                                           &l_fontSheetWidth,
                                                           &l_fontSheetHeight,
                                                           l_pngData,
                                                           l_pngSize,
                                                           LCT_GREY,
                                                           8);

                        if(error != 0)
                        {
                            internal_status = scalable_font_status::failed;
                        }
                        else
                        {
                            // Create font entry:
                            internal_font_data.m_fontName = internal_file_name;
                            internal_font_data.m_boundsYMax = l_maxBoundsY;
                            internal_font_data.m_boundsYMin = l_minBoundsY;
                            internal_font_data.m_lineHeight = 1.1f;
                            internal_font_data.m_pixTexelStride = atl::point2f(1.f / l_fontSheetWidth, 1.f / l_fontSheetHeight);
                            internal_font_data.m_spaceCharacterSize = l_spaceCharSize;

                            // Upload font sheet to video card:
                            internal_font_data.m_texture.alloc();

                            glBindTexture(GL_TEXTURE_2D, internal_font_data.m_texture);
                            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                            glTexImage2D(GL_TEXTURE_2D,
                                         0,
                                         GL_RED_NOM,
                                         l_fontSheetWidth,
                                         l_fontSheetHeight,
                                         0,
                                         GL_RED_NOM,
                                         GL_UNSIGNED_BYTE,
                                         &l_fontSheetData[0]);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering

                            // Fill out all the bookkeeping data to render properly:
                            auto & l_fontChars = internal_font_data.m_fontChars;
                            for(const auto & l_info : l_charInfos)
                            {
                                l_fontChars.emplace_back(l_info.m_char,
                                                         l_info.m_advance,
                                                         l_info.m_offset,
                                                         l_info.m_size,
                                                         atl::box2f(float(l_info.m_sheetYPos) / float(l_fontSheetHeight),
                                                                    float(l_info.m_sheetXPos + l_info.m_sheetWidth) / float(l_fontSheetWidth),
                                                                    float(l_info.m_sheetYPos + l_info.m_sheetHeight) / float(l_fontSheetHeight),
                                                                    float(l_info.m_sheetXPos) / float(l_fontSheetWidth)));
                            }

                            // Free image data
                            free(l_fontSheetData);

                            internal_status = scalable_font_status::ready;
                        }

                        internal_loaded_file_data.free();
                        break;
                    }
                }
                break;
            }
        }
    }
}