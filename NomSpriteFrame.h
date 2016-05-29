

#pragma once

#include "ATLUtil/math2d.h"
#include <string>
#include "atl_graphics_namespace.h"

namespace atl_graphics_namespace_config
{
    struct sprite_frame
    {
    public:
        unsigned int texture_gl_handle;    // Texture sheet GL handle
        atl::box2f texture_coordinates;             // Tex coordinates in texture sheet
        atl::box2f area;                     // Area the image will be drawn in, relative to the draw pos, when untranslated by translation, rotation, or scale

        sprite_frame();

        void set(unsigned int in_glHandle,
                 const atl::box2f & in_texCoords,
                 const atl::box2f & in_area);
    };
}
