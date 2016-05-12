

#pragma once

#include "ATLUtil/math2d.h"
#include "ATLUtil/color.h"

#include "ATF/ATLGLResource.h"
#include "ATF/NomRenderingHeaders.h"
#include "ATF/NomFiles.h"

namespace atl_graphics_namespace_config
{
    struct sprite_frame;
    struct application_folder;
    struct shared_renderer_state;

    struct quad_position
    {
        atl::point2f c1, c2, c3, c4;

        quad_position(atl::box2f in_initBounds) :
            c1(in_initBounds.l, in_initBounds.t),
            c2(in_initBounds.r, in_initBounds.t),
            c3(in_initBounds.r, in_initBounds.b),
            c4(in_initBounds.l, in_initBounds.b)
        {}

        quad_position(const atl::point2f & in_c1,
                      const atl::point2f & in_c2,
                      const atl::point2f & in_c3,
                      const atl::point2f & in_c4) :
            c1(in_c1),
            c2(in_c2),
            c3(in_c3),
            c4(in_c4)
        {}

        quad_position(float c1x, float c1y,
                      float c2x, float c2y,
                      float c3x, float c3y,
                      float c4x, float c4y) :
            c1(c1x, c1y),
            c2(c2x, c2y),
            c3(c3x, c3y),
            c4(c4x, c4y)
        {}

        bool operator != (const quad_position & in_other)
        {
            return  c1 != in_other.c1 ||
                c2 != in_other.c2 ||
                c3 != in_other.c3 ||
                c4 != in_other.c4;
        }
    };

    enum class sprite_renderer_status
    {
        waiting_to_load,
        loading,
        ready,
        failed
    };

    struct sprite_renderer
    {
    private:
        shared_renderer_state & pm_rendererState;

        atl::color_premul pm_colorModulation;
        atl::color pm_colorLerp;

        GLuint pm_currentTexture;
        quad_position pm_currentSpriteCorners;
        atl::box2f pm_currentSpriteTexBounds;
        atl::color_premul pm_currentColorModulation;
        atl::color pm_currentColorLerp;
        atl::box2f pm_currentScreenBounds;

        shader_program_resource pm_program;
        vertex_array_resource pm_vertexArray;
        buffer_resource pm_vertexBuffer;

        sprite_renderer_status internal_status;
        file_data internal_vertex_shader_file;
        file_data internal_fragment_shader_file;

        // Constants
        // Uniform index.
        enum ShaderUniforms
        {
            UNIFORM_SPRITE_CORNERS,
            UNIFORM_SPRITE_TEXCOORDS,
            UNIFORM_SCREEN_DIM,
            UNIFORM_COLOR_MODULATION,
            UNIFORM_COLOR_LERP,
            NUM_UNIFORMS
        };
        int pm_shaderUniforms[NUM_UNIFORMS];

        enum VertexAttributes
        {
            ATTRIBUTE_VERT_INDEXES,
            NUM_ATTRIBUTES
        };

        void internal_configure_vertex_array() const;

        struct SpriteVert
        {
            float c1Component;
            float c2Component;
            float c3Component;
            float c4Component;
        };

    public:
        sprite_renderer(shared_renderer_state & in_rendererState);
        ~sprite_renderer();

        sprite_renderer_status status() const { return internal_status; }
        void incremental_load(const application_folder & in_application_folder);

        quad_position transform(const sprite_frame & in_spriteFrame,
                                const atl::point2f & in_spritePosition,
                                const atl::size2f & in_spriteScale,
                                float in_spriteRotation);

        void renderSprite(const sprite_frame & in_spriteFrame,
                          const atl::point2f & in_spritePosition);

        void renderSprite(const sprite_frame & in_spriteFrame,
                          const atl::point2f & in_spritePosition,
                          const atl::size2f & in_spriteScale);

        void renderSprite(const sprite_frame & in_spriteFrame,
                          const atl::point2f & in_spritePosition,
                          const atl::size2f & in_spriteScale,
                          float in_spriteRotation);

        void renderSprite(const sprite_frame & in_spriteFrame,
                          const quad_position & in_corners);

        void renderSprite(const sprite_frame & in_spriteFrame,
                          const quad_position & in_corners,
                          const atl::box2f & in_texCoords);

        void setColorModulation(const atl::color_premul & in_color);
        void setColorLerp(const atl::color & in_color);

        void clearColorModulation();
        void clearColorLerp();
    };

    struct sprite_scoped_modulation
    {
    public:
        sprite_scoped_modulation(const atl::color & in_color, sprite_renderer & in_renderer) : renderer(in_renderer) { renderer.setColorModulation(in_color); }
        ~sprite_scoped_modulation() { renderer.clearColorModulation(); }
    private:
        sprite_renderer & renderer;
    };

    struct sprite_scoped_lerp
    {
    public:
        sprite_scoped_lerp(const atl::color & in_color, sprite_renderer & in_renderer) : renderer(in_renderer) { renderer.setColorLerp(in_color); }
        ~sprite_scoped_lerp() { renderer.clearColorLerp(); }
    private:
        sprite_renderer & renderer;
    };
}
