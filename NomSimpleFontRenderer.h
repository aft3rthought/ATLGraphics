

#pragma once

#include "ATLUtil/math2d.h"
#include "ATF/ATLGLResource.h"
#include "ATF/NomFiles.h"

#include <string>

namespace atl_graphics_namespace_config
{
    struct application_folder;
    struct scalable_font;
    struct simple_font_profile;
    struct shared_renderer_state;

    enum class simple_font_renderer_status
    {
        waiting_to_load,
        loading,
        ready,
        failed
    };

    namespace glyph_boundary_semantic {
        using underlying_type = unsigned int;
        constexpr underlying_type none                = 0b0;
        constexpr underlying_type character           = 0b1;
        constexpr underlying_type word                = 0b11;
        constexpr underlying_type sentance            = 0b111;
        constexpr underlying_type line                = 0b1111;
    }
    
    struct layout_glyph_type
    {
        atl::size2f size;
        float advance;
        atl::point2f offset;
        glyph_boundary_semantic::underlying_type boundary;
    };
    
    layout_glyph_type * string_to_layout_glyph(const scalable_font & in_font,
                                               const std::string & l_stringToDraw,
                                               layout_glyph_type * in_data_begin,
                                               layout_glyph_type * in_data_end);
    
    void glyph_layout(const atl::box2f & l_drawArea,
                      atl::anchoring l_textAnchoring,
                      float in_numLines,
                      const layout_glyph_type * in_data_begin,
                      const layout_glyph_type * in_data_end,
                      atl::box2f * in_result_bounds_begin,
                      atl::box2f * in_result_bounds_end);

    class simple_font_renderer
    {
    private:
        shared_renderer_state & pm_rendererState;

        // Constants
        // Uniform index.
        enum
        {
            UNIFORM_SCREEN_DIM,
            UNIFORM_TRANSLATION,
            UNIFORM_SAMPLER_DISTANCE_FIELD,
            NUM_UNIFORMS
        };
        int pm_shaderUniforms[NUM_UNIFORMS];

        enum
        {
            ATTRIBUTE_VERT_POSITION,
            ATTRIBUTE_VERT_TEX_COORD,
            ATTRIBUTE_VERT_COLOR,
            ATTRIBUTE_VERT_EDGE,
            ATTRIBUTE_VERT_RADIUS,
            NUM_ATTRIBUTES
        };

        const static int pcsm_maxLayersPerPass = 4;
        const static int pcsm_maxCharactersPerPass = 256;
        const static int pcsm_maxCharsForBuffer = pcsm_maxCharactersPerPass * pcsm_maxLayersPerPass;
        const static int pcsm_maxVertsPerPass = pcsm_maxCharsForBuffer * 4;
        const static int pcsm_indicesPerCharacter = 2 * 3;
        const static int pcsm_maxIndicesPerPass = pcsm_maxCharsForBuffer * pcsm_indicesPerCharacter;
        const static int pcsm_numBuffers = 2;

        struct Vertex {
            GLfloat x;
            GLfloat y;
            GLfloat u;
            GLfloat v;
            GLfloat r;
            GLfloat g;
            GLfloat b;
            GLfloat a;
            GLfloat edge;
            GLfloat radius;
        };
        Vertex pm_vertexData[pcsm_maxVertsPerPass];
        GLushort pm_indexData[pcsm_maxIndicesPerPass];
        shader_program_resource pm_glProgram;
        buffer_resource pm_glIndexBuffer;
        vertex_array_resource pm_glVertexArray[pcsm_numBuffers];
        buffer_resource pm_glVertexBuffer[pcsm_numBuffers];
        int pm_currentVertexBuffer;

        simple_font_renderer_status internal_status;
        file_data internal_vertex_shader_file;
        file_data internal_fragment_shader_file;

        void p_drawBufferUsingProfile(const simple_font_profile & in_profile,
                                      const scalable_font & in_font,
                                      const atl::point2f & in_position,
                                      float in_scale,
                                      unsigned int in_numPrims);

        void internal_configure_vertex_array(const int in_buffer_idx);

    public:
        simple_font_renderer(shared_renderer_state & in_rendererState);
        ~simple_font_renderer();

        simple_font_renderer_status status() const { return internal_status; }
        void incremental_load(const application_folder & in_application_folder);

        void render(const simple_font_profile & in_profile,
                    const scalable_font & l_font,
                    const atl::box2f & in_drawArea,
                    atl::anchoring in_textAnchoring,
                    const std::string & in_stringToDraw,
                    int in_numLines = 1);
    };
}
