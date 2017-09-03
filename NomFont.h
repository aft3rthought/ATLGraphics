

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

    struct scalable_font_char
    {
    public:
        uint32_t m_char;
        float m_advance;
        atl::point2f m_offset;
        atl::size2f m_size;
        atl::box2f m_texBounds;

        scalable_font_char(char in_char,
                           float in_advance,
                           const atl::point2f & in_offset,
                           const atl::size2f & in_size,
                           const atl::box2f & in_texBounds);
    };

    struct scalable_font_data
    {
        std::string m_fontName;
        texture_resource m_texture;
        std::vector<scalable_font_char> m_fontChars;
        atl::point2f m_pixTexelStride;
        float m_boundsYMin;
        float m_boundsYMax;
        float m_lineHeight;
        float field_bloom;
    };

    enum class scalable_font_status
    {
        waiting_to_load,
        loading,
        ready,
        failed
    };

    struct scalable_font
    {
    public:
        scalable_font(const char * in_file_name);
        ~scalable_font();

        const scalable_font_data & font_data() const { return internal_font_data; }
        scalable_font_status status() const { return internal_status; }
        void incremental_load(const application_folder & in_application_folder);

    private:
        std::string internal_file_name;
        scalable_font_data internal_font_data;
        std::atomic<scalable_font_status> internal_status;
        file_data internal_loaded_file_data;
    };
}
