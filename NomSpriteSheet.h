

#pragma once

#include <vector>
#include "ATF/NomSpriteFrame.h"
#include "ATF/NomFiles.h"
#include "ATF/texture_filtering_mode.h"

namespace atl_graphics_namespace_config
{
    struct application_folder;

    enum class sprite_sheet_status
    {
        waiting_to_load,
        loading,
        ready,
        failed
    };

    struct sprite_sheet
    {
        std::vector<unsigned int> textures;
        std::vector<sprite_frame> sprites;

        sprite_sheet(const char * in_sheet_file_path, const texture_filtering_mode in_filtering_mode);
        ~sprite_sheet();

        sprite_sheet_status status() const { return internal_status; }
        void incremental_load(const application_folder & in_application_folder);

    private:
        texture_filtering_mode internal_texture_filtering_mode;
        std::string internal_sheet_file_path;
        sprite_sheet_status internal_status;
        file_data internal_loaded_file_data;
    };
}
