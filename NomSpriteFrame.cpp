

#include "./NomSpriteFrame.h"

namespace atl_graphics_namespace_config
{
    sprite_frame::sprite_frame() : texture_gl_handle(0) {}

    void sprite_frame::set(unsigned int in_glHandle, const atl::box2f & in_texCoords, const atl::box2f & in_area)
    {
        texture_gl_handle = in_glHandle;
        area = in_area;
        texture_coordinates = in_texCoords;
    }
}