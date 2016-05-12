

#pragma once

#include "ATLUtil/math2d.h"
#include "ATLUtil/color.h"
#include "ATF/ATLGLResource.h"
#include "ATF/NomFiles.h"

#include <string>
#include <atomic>

namespace atl_graphics_namespace_config
{
    struct bitmap_font;
    struct shared_renderer_state;

    enum class bitmap_font_renderer_status
    {
        waiting_to_load,
        loading,
        ready,
        failed
    };

    struct bitmap_font_renderer
    {
    private:
        shared_renderer_state & internal_shared_renderer_state;

        // Constants
        // Uniform index.
        enum
        {
            UNIFORM_SCREEN_DIM,
            UNIFORM_TRANSLATION,
            UNIFORM_SAMPLER_TEXTURE,
            UNIFORM_FONT_COLOR,
            NUM_UNIFORMS
        };
        int pm_shaderUniforms[NUM_UNIFORMS];

        enum
        {
            ATTRIBUTE_VERT_POSITION,
            ATTRIBUTE_VERT_TEX_COORD,
            NUM_ATTRIBUTES
        };

        const static int pcsm_maxCharactersPerPass = 256;
        const static int pcsm_maxCharsForBuffer = pcsm_maxCharactersPerPass;
        const static int pcsm_maxVertsPerPass = pcsm_maxCharsForBuffer * 4;
        const static int pcsm_indicesPerCharacter = 2 * 3;
        const static int pcsm_maxIndicesPerPass = pcsm_maxCharsForBuffer * pcsm_indicesPerCharacter;
        const static int pcsm_numBuffers = 4;

        struct Vertex {
            GLfloat x;
            GLfloat y;
            GLfloat u;
            GLfloat v;
        };

        Vertex pm_vertexData[pcsm_maxVertsPerPass];
        GLushort pm_indexData[pcsm_maxIndicesPerPass];
        shader_program_resource pm_glProgram;
        buffer_resource pm_glIndexBuffer;
        vertex_array_resource pm_glVertexArray[pcsm_numBuffers];
        buffer_resource pm_glVertexBuffer[pcsm_numBuffers];
        int pm_currentVertexBuffer;
        void internal_configure_vertex_array(int in_idx_buffer);
        file_data internal_vertex_shader_file;
        file_data internal_fragment_shader_file;
        std::atomic<bitmap_font_renderer_status> internal_status;

        void p_drawBufferUsingProfile(const bitmap_font & in_font,
                                      const atl::point2f & in_position,
                                      unsigned int in_numPrims,
                                      const atl::color & in_color);

    public:
        bitmap_font_renderer(shared_renderer_state & in_rendererState);
        ~bitmap_font_renderer();
        bitmap_font_renderer_status status() const { return internal_status; }
        void incremental_load(const application_folder & in_application_folder);

        void render(const bitmap_font & l_font,
                    const atl::box2f & in_drawArea,
                    atl::anchoring in_textAnchoring,
                    const std::string & in_stringToDraw,
                    const atl::color & in_color,
                    int in_numLines = 1);

        atl::point2f drawCharacter(const bitmap_font & l_font,
                                   const char in_character,
                                   const atl::point2f & in_position,
                                   const atl::color & in_color);
    };
}
