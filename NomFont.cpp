#include "./NomFont.h"

#include "ATLUtil/numeric_util.h"
#include "ATLUtil/bit_string.h"
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
                        auto file_data = internal_loaded_file_data.data();
                        atl::input_bit_string_type bit_string(file_data.data(), file_data.data() + file_data.size());

                        struct l_CharInfo
                        {
                            uint32_t m_char;
                            float m_advance;
                            atl::point2f m_offset;
                            atl::size2f m_size;
                            uint32_t m_sheetXPos;
                            uint32_t m_sheetYPos;
                            uint32_t m_sheetWidth;
                            uint32_t m_sheetHeight;
                        };

                        float field_bloom = 0.f;
                        atl::bit_string_read_value(bit_string, field_bloom);
                        
                        int fontSheetWidthPower, fontSheetHeightPower;
                        atl::bit_string_read_ranged_integer<int>(bit_string, fontSheetWidthPower, 0, 12);
                        atl::bit_string_read_ranged_integer<int>(bit_string, fontSheetHeightPower, 0, 12);
                        uint32_t fontSheetWidth = 1 << fontSheetWidthPower;
                        uint32_t fontSheetHeight = 1 << fontSheetHeightPower;
                        
                        int l_numCharacters;
                        atl::bit_string_read_chunked_integer<int>(bit_string, l_numCharacters, 7);

                        std::vector<l_CharInfo> l_charInfos;
                        l_charInfos.reserve(l_numCharacters);

                        float l_minBoundsY = std::numeric_limits<float>::max();
                        float l_maxBoundsY = -std::numeric_limits<float>::max();
                        while(l_numCharacters-- > 0)
                        {
                            l_charInfos.emplace_back();
                            atl::bit_string_read_chunked_integer<uint32_t>(bit_string, l_charInfos.back().m_char, 8);
                            atl::bit_string_read_value(bit_string, l_charInfos.back().m_advance);
                            atl::bit_string_read_value(bit_string, l_charInfos.back().m_offset.x);
                            atl::bit_string_read_value(bit_string, l_charInfos.back().m_offset.y);
                            atl::bit_string_read_value(bit_string, l_charInfos.back().m_size.w);
                            atl::bit_string_read_value(bit_string, l_charInfos.back().m_size.h);
                            atl::bit_string_read_ranged_integer<uint32_t>(bit_string, l_charInfos.back().m_sheetXPos, 0, fontSheetWidth);
                            atl::bit_string_read_ranged_integer<uint32_t>(bit_string, l_charInfos.back().m_sheetYPos, 0, fontSheetHeight);
                            atl::bit_string_read_ranged_integer<uint32_t>(bit_string, l_charInfos.back().m_sheetWidth, 0, fontSheetWidth);
                            atl::bit_string_read_ranged_integer<uint32_t>(bit_string, l_charInfos.back().m_sheetHeight, 0, fontSheetHeight);

                            l_minBoundsY = std::min(l_minBoundsY, l_charInfos.back().m_offset.y);
                            l_maxBoundsY = std::max(l_maxBoundsY, l_charInfos.back().m_offset.y + l_charInfos.back().m_size.h);
                        }

                        int32_t l_pngSizeInt32;
                        atl::bit_string_read_value(bit_string, l_pngSizeInt32);
                        size_t l_pngSize = l_pngSizeInt32;
                        atl::bit_string_skip_to_next_byte(bit_string);
                        const unsigned char * l_pngData = bit_string.ptr;
                        bit_string.ptr += l_pngSize;

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
                            internal_font_data.field_bloom = field_bloom;
                            internal_font_data.m_boundsYMax = l_maxBoundsY;
                            internal_font_data.m_boundsYMin = l_minBoundsY;
                            internal_font_data.m_lineHeight = 1.1f;
                            internal_font_data.m_pixTexelStride = atl::point2f(1.f / l_fontSheetWidth, 1.f / l_fontSheetHeight);

                            // Upload font sheet to video card:
                            internal_font_data.m_texture.alloc();

                            glBindTexture(GL_TEXTURE_2D, internal_font_data.m_texture);
                            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                            glTexImage2D(GL_TEXTURE_2D,
                                         0,
                                         GL_RED_NOM,
                                         atl::integer_cast<GLsizei>(l_fontSheetWidth, [](){}),
                                         atl::integer_cast<GLsizei>(l_fontSheetHeight, [](){}),
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
