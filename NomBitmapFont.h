

#pragma once

#include "ATF/ATLGLResource.h"
#include "ATLUtil/math2d.h"
#include "ATF/NomFiles.h"

#include <vector>
#include <string>
#include <atomic>

namespace atl_graphics_namespace_config
{
    struct application_folder;

    struct bitmap_font_char
    {
    public:
        char m_char;
        float m_advance;
        atl::point2f m_offset;
        atl::size2f m_size;
        atl::box2f m_texBounds;

        bitmap_font_char(char in_char,
                         float in_advance,
                         const atl::point2f & in_offset,
                         const atl::size2f & in_size,
                         const atl::box2f & in_texBounds);
    };

    struct bitmap_font_data
    {
        std::string m_fontName;
        float m_spaceCharacterSize;
        texture_resource m_texture;
        std::vector<bitmap_font_char> m_fontChars;
        atl::point2f m_pixTexelStride;
        int32_t m_fontSize;
        float m_boundsYMin;
        float m_boundsYMax;
        float m_lineHeight;
    };

    enum class bitmap_font_status
    {
        waiting_to_load,
        loading,
        ready,
        failed
    };

    struct bitmap_font
    {
    public:
        bitmap_font(const char * in_file_name);
        ~bitmap_font();

        const bitmap_font_data & font_data() const { return internal_font_data; }
        bitmap_font_status status() const { return internal_status; }
        void incremental_load(const application_folder & in_application_folder);

    private:
        std::string internal_file_name;
        bitmap_font_data internal_font_data;
        std::atomic<bitmap_font_status> internal_status;
        file_data internal_loaded_file_data;
    };
}
