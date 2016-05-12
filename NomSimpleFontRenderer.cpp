

#include "./NomSimpleFontRenderer.h"

#include "ATLUtil/array_util.h"
#include "ATLUtil/debug_break.h"
#include "ATF/NomFiles.h"
#include "ATF/NomFont.h"
#include "ATF/NomSimpleFontProfile.h"
#include "ATF/RenderUtil.h"
#include "ATF/NomRendererState.h"

namespace atl_graphics_namespace_config
{
    simple_font_renderer::simple_font_renderer(shared_renderer_state & in_rendererState) :
        pm_currentVertexBuffer(0),
        pm_rendererState(in_rendererState),
        internal_status(simple_font_renderer_status::waiting_to_load)
    {}

    void simple_font_renderer::incremental_load(const application_folder & in_application_folder)
    {
        switch(internal_status)
        {
            case simple_font_renderer_status::ready:
            case simple_font_renderer_status::failed:
                break;
            case simple_font_renderer_status::waiting_to_load:
            {
                internal_vertex_shader_file.load(in_application_folder, "SimpleFont.vsh");
                internal_fragment_shader_file.load(in_application_folder, "SimpleFont.fsh");
                internal_status = simple_font_renderer_status::loading;
                break;
            }
            case simple_font_renderer_status::loading:
            {
                auto l_vsh_status = internal_vertex_shader_file.status();
                auto l_fsh_status = internal_fragment_shader_file.status();
                if((l_vsh_status != file_data_status::loading &&
                    l_vsh_status != file_data_status::ready) ||
                    (l_fsh_status != file_data_status::loading &&
                     l_fsh_status != file_data_status::ready))
                {
                    internal_status = simple_font_renderer_status::failed;
                }
                else if(l_vsh_status == file_data_status::ready &&
                        l_fsh_status == file_data_status::ready)
                {
                    GLuint l_vertex_shader = 0, l_fragment_shader = 0;

                    if(!compile_shader(internal_vertex_shader_file.data(), &l_vertex_shader, GL_VERTEX_SHADER) ||
                       !compile_shader(internal_fragment_shader_file.data(), &l_fragment_shader, GL_FRAGMENT_SHADER))
                    {
                        atl_break_debug("Shader failed to compile");
                        internal_status = simple_font_renderer_status::failed;
                    }
                    else
                    {
                        // Index data:
                        {
                            unsigned l_vertIdx = 0;
                            for(unsigned int idx = 0; idx < pcsm_maxIndicesPerPass; idx += pcsm_indicesPerCharacter)
                            {
                                pm_indexData[idx + 0] = l_vertIdx + 0;
                                pm_indexData[idx + 1] = l_vertIdx + 1;
                                pm_indexData[idx + 2] = l_vertIdx + 2;
                                pm_indexData[idx + 3] = l_vertIdx + 1;
                                pm_indexData[idx + 4] = l_vertIdx + 2;
                                pm_indexData[idx + 5] = l_vertIdx + 3;
                                l_vertIdx += 4;
                            }
                        }

                        // Do GL stuff to set up the vertex buffer:    
                        for(int i = 0; i < pcsm_numBuffers; i++)
                        {
                            pm_glVertexArray[i].alloc();
                            pm_glVertexBuffer[i].alloc();
                        }
                        pm_glIndexBuffer.alloc();

                        for(int idx_buffer = 0; idx_buffer < pcsm_numBuffers; idx_buffer++)
                        {
                            pm_glVertexArray[idx_buffer].bind();
                            internal_configure_vertex_array(idx_buffer);
                            glBufferData(GL_ARRAY_BUFFER, atl::c_array_byte_length(pm_vertexData), NULL, GL_DYNAMIC_DRAW);
                            check_gl_errors();
                            glBufferData(GL_ELEMENT_ARRAY_BUFFER, atl::c_array_byte_length(pm_indexData), &pm_indexData[0], GL_STATIC_DRAW);
                            check_gl_errors();
                        }
                        atl::c_array_last(pm_glVertexArray).unbind();

                        // Create shader program.
                        pm_glProgram.alloc();

                        // Attach vertex shader to program.
                        glAttachShader(pm_glProgram, l_vertex_shader);

                        // Attach fragment shader to program.
                        glAttachShader(pm_glProgram, l_fragment_shader);

                        // Bind attribute locations.
                        // This needs to be done prior to linking.
                        glBindAttribLocation(pm_glProgram, ATTRIBUTE_VERT_POSITION, "v_position");
                        glBindAttribLocation(pm_glProgram, ATTRIBUTE_VERT_TEX_COORD, "v_texCoord");
                        glBindAttribLocation(pm_glProgram, ATTRIBUTE_VERT_INDEX, "v_index");

                        // Link program.
                        if(link_program(pm_glProgram))
                        {
                            // Get uniform locations.
                            pm_shaderUniforms[UNIFORM_SCREEN_DIM] = glGetUniformLocation(pm_glProgram, "u_screenBounds");
                            pm_shaderUniforms[UNIFORM_TRANSLATION] = glGetUniformLocation(pm_glProgram, "u_translation");
                            pm_shaderUniforms[UNIFORM_EDGES] = glGetUniformLocation(pm_glProgram, "u_edges");
                            pm_shaderUniforms[UNIFORM_COLORS] = glGetUniformLocation(pm_glProgram, "u_colors");
                            pm_shaderUniforms[UNIFORM_ANTIALIASING_RADIUS] = glGetUniformLocation(pm_glProgram, "u_antialiasingRadius");

                            pm_shaderUniforms[UNIFORM_SAMPLER_DISTANCE_FIELD] = glGetUniformLocation(pm_glProgram, "s_distanceField");

                            internal_status = simple_font_renderer_status::ready;
                        }
                        else
                        {
                            atl_break_debug("Shader failed to link");
                            pm_glProgram.free();
                            internal_status = simple_font_renderer_status::failed;
                        }
                    }

                    internal_vertex_shader_file.free();
                    internal_fragment_shader_file.free();

                    // Release vertex and fragment shaders.
                    if(l_vertex_shader != 0)
                    {
                        glDetachShader(pm_glProgram, l_vertex_shader);
                        glDeleteShader(l_vertex_shader);
                    }
                    if(l_fragment_shader != 0)
                    {
                        glDetachShader(pm_glProgram, l_fragment_shader);
                        glDeleteShader(l_fragment_shader);
                    }
                }
                break;
            }
        }
    }

    simple_font_renderer::~simple_font_renderer()
    {
        pm_glIndexBuffer.free();
        pm_glProgram.free();
        for(int i = 0; i < pcsm_numBuffers; i++)
        {
            pm_glVertexArray[i].free();
            pm_glVertexBuffer[i].free();
        }
    }

    void simple_font_renderer::internal_configure_vertex_array(const int in_buffer_idx)
    {
        glBindBuffer(GL_ARRAY_BUFFER, pm_glVertexBuffer[in_buffer_idx]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pm_glIndexBuffer);

        glEnableVertexAttribArray(ATTRIBUTE_VERT_POSITION);
        glVertexAttribPointer(ATTRIBUTE_VERT_POSITION, 2, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset_start(Vertex));

        glEnableVertexAttribArray(ATTRIBUTE_VERT_TEX_COORD);
        glVertexAttribPointer(ATTRIBUTE_VERT_TEX_COORD, 2, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset(Vertex, u));

        glEnableVertexAttribArray(ATTRIBUTE_VERT_INDEX);
        glVertexAttribPointer(ATTRIBUTE_VERT_INDEX, 1, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset(Vertex, index));
    }

    void simple_font_renderer::render(const simple_font_profile & in_profile,
                                      const scalable_font & in_font,
                                      const atl::box2f & l_drawArea,
                                      atl::anchoring l_textAnchoring,
                                      const std::string & l_stringToDraw,
                                      int in_numLines)
    {
        const auto & l_font = in_font.font_data();

        if(pm_rendererState.setAsCurrentRenderer(this))
        {
            glUseProgram(pm_glProgram);

            glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                        pm_rendererState.m_currentBounds.l,
                        pm_rendererState.m_currentBounds.b,
                        pm_rendererState.m_currentBounds.width(),
                        pm_rendererState.m_currentBounds.height());
        }

        // Measure string first:
        struct l_WhitespaceSplitUnit
        {
            atl::point2f m_drawPos;

            atl::box2f m_bounds;

            struct l_Char
            {
                int m_fontElementIdx;
                float m_scale;
                float m_kerningScale;

                l_Char(int in_fontElementIdx, float in_scale, float in_kerningScale) :
                    m_fontElementIdx(in_fontElementIdx),
                    m_scale(in_scale),
                    m_kerningScale(in_kerningScale)
                {}
            };

            std::vector<l_Char> m_chars;
            float m_advance;
            float m_whitespaceAdvance;
            int m_line;
            bool m_linebreak;

            l_WhitespaceSplitUnit() :
                m_advance(0.f),
                m_linebreak(false)
            {
                m_bounds = atl::box2f::MaxInvertedBounds;
            }
        };

        std::vector<l_WhitespaceSplitUnit> l_whitespaceUnits;

        float l_kerningScaleValue = 1.f;
        float l_subStringScaleValue = 1.f;
        float l_totalLength = 0.f;
        bool l_useManualLinebreaking = false;
        {
            atl::point2f l_curDrawPos(0.f, 0.f);
            l_whitespaceUnits.emplace_back();

            bool l_inTag = false;
            std::string l_tagString;
            const char * l_str = l_stringToDraw.c_str();
            for(unsigned int idx = 0; idx < l_stringToDraw.length(); idx++, l_str++)
            {
                if(l_inTag)
                {
                    if(*l_str == '>')
                    {
                        if(l_tagString[0] == 's' && l_tagString[1] == ':')
                        {
                            std::string l_newScale;
                            for(unsigned int idx_scaleChar = 2; idx_scaleChar < l_tagString.length(); idx_scaleChar++)
                                l_newScale += l_tagString[idx_scaleChar];

                            l_subStringScaleValue = std::stof(l_newScale);
                        }
                        else if(l_tagString[0] == 'k' && l_tagString[1] == ':')
                        {
                            std::string l_newScale;
                            for(unsigned int idx_scaleChar = 2; idx_scaleChar < l_tagString.length(); idx_scaleChar++)
                                l_newScale += l_tagString[idx_scaleChar];

                            l_kerningScaleValue = std::stof(l_newScale);
                        }

                        l_inTag = false;
                    }
                    else
                    {
                        l_tagString += *l_str;
                    }
                }
                else
                {
                    if(*l_str == ' ')
                    {
                        float l_scaledSpaceAdvance = l_font.m_spaceCharacterSize * l_subStringScaleValue * l_kerningScaleValue;

                        l_whitespaceUnits.back().m_whitespaceAdvance = l_scaledSpaceAdvance;

                        l_totalLength += l_scaledSpaceAdvance;

                        l_curDrawPos = atl::point2f(0.f, 0.f);

                        if(l_whitespaceUnits.back().m_bounds != atl::box2f::MaxInvertedBounds)
                            l_whitespaceUnits.emplace_back();
                    }
                    else if(*l_str == '\n')
                    {
                        l_useManualLinebreaking = true;

                        l_whitespaceUnits.back().m_whitespaceAdvance = 0.f;
                        l_whitespaceUnits.back().m_linebreak = true;

                        l_curDrawPos = atl::point2f(0.f, 0.f);

                        if(l_whitespaceUnits.back().m_bounds != atl::box2f::MaxInvertedBounds)
                            l_whitespaceUnits.emplace_back();
                    }
                    else if(*l_str == '<')
                    {
                        l_inTag = true;
                        l_tagString = "";
                    }
                    else
                    {
                        for(unsigned int idx_fontChar = 0; idx_fontChar < l_font.m_fontChars.size(); idx_fontChar++)
                        {
                            const auto & l_fontChar = l_font.m_fontChars[idx_fontChar];
                            if(l_fontChar.m_char == *l_str)
                            {
                                atl::point2f l_pt1(l_curDrawPos.x + l_fontChar.m_offset.x * l_subStringScaleValue,
                                                   l_curDrawPos.y);

                                atl::point2f l_pt2(l_curDrawPos.x + l_fontChar.m_offset.x * l_subStringScaleValue + l_fontChar.m_size.w * l_subStringScaleValue,
                                                   l_curDrawPos.y + 1.f * l_subStringScaleValue);

                                l_whitespaceUnits.back().m_bounds.include(l_pt1);
                                l_whitespaceUnits.back().m_bounds.include(l_pt2);
                                l_whitespaceUnits.back().m_chars.emplace_back(idx_fontChar, l_subStringScaleValue, l_kerningScaleValue);

                                float l_advanceAmountForChar = l_fontChar.m_advance * l_subStringScaleValue * l_kerningScaleValue;

                                l_whitespaceUnits.back().m_advance += l_advanceAmountForChar;
                                l_totalLength += l_advanceAmountForChar;
                                l_curDrawPos.x += l_advanceAmountForChar;

                                break;
                            }
                        }
                    }
                }
            }
        }

        // - Scale starts at height / num lines:
        float l_drawScale = l_drawArea.height() / in_numLines;

        // Now do the layout pass:
        atl::box2f l_drawBounds = atl::box2f::MaxInvertedBounds;
        {
            float l_maxLineLength = std::max(l_totalLength / in_numLines, l_drawArea.width() / l_drawScale);

            int l_currentLineIdx = 0;

            atl::point2f l_curPos(0.f, 0.f);
            for(auto & l_whitespaceUnit : l_whitespaceUnits)
            {
                if(l_curPos.x > 0.f &&
                   !l_useManualLinebreaking &&
                   l_whitespaceUnit.m_advance + l_curPos.x > l_maxLineLength &&
                   l_currentLineIdx < in_numLines - 1)
                {
                    l_curPos.x = 0.f;
                    l_curPos.y -= 1.f * l_font.m_lineHeight;
                    l_currentLineIdx++;
                }

                l_whitespaceUnit.m_line = l_currentLineIdx;
                l_whitespaceUnit.m_drawPos = l_curPos;
                l_whitespaceUnit.m_bounds += l_curPos;
                l_drawBounds.include(l_whitespaceUnit.m_bounds);

                l_curPos.x += l_whitespaceUnit.m_advance;
                l_curPos.x += l_whitespaceUnit.m_whitespaceAdvance;

                if(l_whitespaceUnit.m_linebreak)
                {
                    l_curPos.x = 0.f;
                    l_curPos.y -= 1.f * l_font.m_lineHeight;
                    l_currentLineIdx++;
                }
            }

            {
                float l_yAdjustment = (float)l_currentLineIdx  * l_font.m_lineHeight;

                for(float l_line = 0.f; l_line < l_currentLineIdx + 1; l_line++)
                {
                    float l_lineBegin = 0.f;
                    float l_lineEnd = 0.f;
                    {
                        bool l_inLine = false;
                        for(auto & l_whitespaceUnit : l_whitespaceUnits)
                        {
                            if(l_whitespaceUnit.m_line == l_line)
                            {
                                if(l_inLine)
                                {
                                    l_lineEnd = l_whitespaceUnit.m_bounds.r;
                                }
                                else
                                {
                                    l_inLine = true;
                                    l_lineBegin = l_whitespaceUnit.m_bounds.l;
                                    l_lineEnd = l_whitespaceUnit.m_bounds.r;
                                }
                            }
                            else
                            {
                                if(l_inLine)
                                {
                                    break;
                                }
                            }
                        }
                    }

                    float l_lineWidth = l_lineEnd - l_lineBegin;

                    // Adjust for formatting:
                    float l_xAdjustment;
                    switch(l_textAnchoring)
                    {
                        case atl::anchoring::top_left:
                        case atl::anchoring::center_left:
                        case atl::anchoring::bottom_left:

                            l_xAdjustment = 0.f;
                            break;
                        case atl::anchoring::top_center:
                        case atl::anchoring::centered:
                        case atl::anchoring::bottom_center:

                            l_xAdjustment = (l_drawBounds.width() - l_lineWidth) * 0.5f;
                            break;
                        case atl::anchoring::top_right:
                        case atl::anchoring::center_right:
                        case atl::anchoring::bottom_right:

                            l_xAdjustment = l_drawBounds.width() - l_lineWidth;
                            break;
                    };

                    //
                    for(auto & l_whitespaceUnit : l_whitespaceUnits)
                    {
                        if(l_whitespaceUnit.m_line == l_line)
                        {
                            l_whitespaceUnit.m_drawPos.x += l_xAdjustment;
                            l_whitespaceUnit.m_drawPos.y += l_yAdjustment;
                        }
                    }
                }
            }
        }

        // Now place drawn bounds in the draw area:
        l_drawBounds.x.scale(l_drawScale);
        l_drawBounds.y.scale(l_drawScale);

        // Now do the scale pass:
        // Now do final scale for width:
        if(l_drawBounds.width() > l_drawArea.width())
        {
            float l_rescaleVal = l_drawArea.width() / l_drawBounds.width();
            l_drawScale *= l_rescaleVal;
            l_drawBounds.x.scale(l_rescaleVal);
            l_drawBounds.y.scale(l_rescaleVal);
        }

        // And do final scale for height:
        if(l_drawBounds.height() > l_drawArea.height())
        {
            float l_rescaleVal = l_drawArea.height() / l_drawBounds.height();
            l_drawScale *= l_rescaleVal;
            l_drawBounds.x.scale(l_rescaleVal);
            l_drawBounds.y.scale(l_rescaleVal);
        }

        // Apply gravity:
        l_drawBounds = l_drawArea.get_sub_bounds_with_size(l_drawBounds.size(), l_textAnchoring);

        // Now draw string:
        {
            simple_font_renderer::Vertex * l_vertexData = pm_vertexData;
            simple_font_renderer::Vertex * l_end = pm_vertexData + atl::c_array_len(pm_vertexData);
            unsigned int l_numPrims = 0;

            for(int idx_layer = 0; idx_layer < pcsm_maxLayersPerPass; idx_layer++)
            {
                for(const auto & l_whitespaceUnit : l_whitespaceUnits)
                {
                    atl::point2f l_curDrawPos = l_whitespaceUnit.m_drawPos * l_drawScale;

                    for(const auto & l_layoutChar : l_whitespaceUnit.m_chars)
                    {
                        const auto & l_fontChar = l_font.m_fontChars[l_layoutChar.m_fontElementIdx];

                        atl::point2f l_charDrawPos = l_curDrawPos;
                        l_charDrawPos += l_fontChar.m_offset * l_drawScale  * l_layoutChar.m_scale;

                        atl::box2f l_charDrawBounds(l_charDrawPos.y + l_fontChar.m_size.h * l_drawScale * l_layoutChar.m_scale,
                                                    l_charDrawPos.x + l_fontChar.m_size.w * l_drawScale * l_layoutChar.m_scale,
                                                    l_charDrawPos.y,
                                                    l_charDrawPos.x);

                        if(l_vertexData + 16 < l_end)
                        {
                            l_numPrims += 2;

                            // Bottom left:
                            (*l_vertexData).x = l_charDrawBounds.l;
                            (*l_vertexData).y = l_charDrawBounds.b;
                            (*l_vertexData).u = l_fontChar.m_texBounds.l;
                            (*l_vertexData).v = l_fontChar.m_texBounds.b;
                            (*l_vertexData).index = (float)idx_layer;
                            l_vertexData++;

                            // Bottom right:
                            (*l_vertexData).x = l_charDrawBounds.r;
                            (*l_vertexData).y = l_charDrawBounds.b;
                            (*l_vertexData).u = l_fontChar.m_texBounds.r;
                            (*l_vertexData).v = l_fontChar.m_texBounds.b;
                            (*l_vertexData).index = (float)idx_layer;
                            l_vertexData++;

                            // Top left:
                            (*l_vertexData).x = l_charDrawBounds.l;
                            (*l_vertexData).y = l_charDrawBounds.t;
                            (*l_vertexData).u = l_fontChar.m_texBounds.l;
                            (*l_vertexData).v = l_fontChar.m_texBounds.t;
                            (*l_vertexData).index = (float)idx_layer;
                            l_vertexData++;

                            // Top right:
                            (*l_vertexData).x = l_charDrawBounds.r;
                            (*l_vertexData).y = l_charDrawBounds.t;
                            (*l_vertexData).u = l_fontChar.m_texBounds.r;
                            (*l_vertexData).v = l_fontChar.m_texBounds.t;
                            (*l_vertexData).index = (float)idx_layer;
                            l_vertexData++;
                        }

                        l_curDrawPos.x += l_fontChar.m_advance * l_drawScale * l_layoutChar.m_scale * l_layoutChar.m_kerningScale;
                    }
                }
            }

            if(l_vertexData != pm_vertexData)
            {
                p_drawBufferUsingProfile(in_profile,
                                         in_font,
                                         atl::point2f(l_drawBounds.l, l_drawBounds.b),
                                         l_drawScale,
                                         l_numPrims);
            }
        }
    }

    void simple_font_renderer::p_drawBufferUsingProfile(const simple_font_profile & in_profile,
                                                        const scalable_font & in_scalable_font,
                                                        const atl::point2f & in_position,
                                                        float in_scale,
                                                        unsigned int in_numPrims)
    {
        const auto & l_font_data = in_scalable_font.font_data();

        if(pm_glVertexArray[pm_currentVertexBuffer].bind())
        {
            glBindBuffer(GL_ARRAY_BUFFER, pm_glVertexBuffer[pm_currentVertexBuffer]);
        }
        else
        {
            internal_configure_vertex_array(pm_currentVertexBuffer);
        }

        // Upload vertex data:
        glBufferSubData(GL_ARRAY_BUFFER, 0, atl::c_array_len(pm_vertexData) * sizeof(simple_font_renderer::Vertex), pm_vertexData);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, l_font_data.m_texture);

        // Shared uniforms:
        glUniform2f(pm_shaderUniforms[UNIFORM_TRANSLATION], in_position.x, in_position.y);
        glUniform1f(pm_shaderUniforms[UNIFORM_ANTIALIASING_RADIUS], 0.1f * (30.f / in_scale));
        glUniform1i(pm_shaderUniforms[UNIFORM_SAMPLER_DISTANCE_FIELD], 0);

        glUniform1fv(pm_shaderUniforms[UNIFORM_EDGES], 4, in_profile.m_edges);

        {
            GLfloat l_colors[4][4]{
                {in_profile.m_colors[0].r, in_profile.m_colors[0].g, in_profile.m_colors[0].b, in_profile.m_colors[0].a},
                {in_profile.m_colors[1].r, in_profile.m_colors[1].g, in_profile.m_colors[1].b, in_profile.m_colors[1].a},
                {in_profile.m_colors[2].r, in_profile.m_colors[2].g, in_profile.m_colors[2].b, in_profile.m_colors[2].a},
                {in_profile.m_colors[3].r, in_profile.m_colors[3].g, in_profile.m_colors[3].b, in_profile.m_colors[3].a},
            };
            glUniform4fv(pm_shaderUniforms[UNIFORM_COLORS], 4, &(l_colors[0][0]));
        }

        glDrawElements(GL_TRIANGLES, in_numPrims * 3, GL_UNSIGNED_SHORT, (void*)0);

        // Swap buffers:
        pm_currentVertexBuffer = (pm_currentVertexBuffer + 1) % pcsm_numBuffers;
    }


    atl::point2f simple_font_renderer::drawCharacter(const simple_font_profile & in_profile,
                                                     const scalable_font & in_scalable_font,
                                                     const char in_character,
                                                     const atl::point2f & in_position,
                                                     const float in_size)
    {
        const auto & l_font_data = in_scalable_font.font_data();

        if(pm_rendererState.setAsCurrentRenderer(this))
        {
            glUseProgram(pm_glProgram);

            glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                        pm_rendererState.m_currentBounds.l,
                        pm_rendererState.m_currentBounds.b,
                        pm_rendererState.m_currentBounds.width(),
                        pm_rendererState.m_currentBounds.height());
        }

        //
        for(const auto & l_fontChar : l_font_data.m_fontChars)
        {
            if(l_fontChar.m_char == in_character)
            {
                atl::box2f l_charDrawBounds(l_fontChar.m_offset.y + l_fontChar.m_size.h * in_size,
                                            l_fontChar.m_offset.x + l_fontChar.m_size.w * in_size,
                                            l_fontChar.m_offset.y,
                                            l_fontChar.m_offset.x);

                simple_font_renderer::Vertex * l_vertexData = pm_vertexData;

                for(int idx_layer = 0; idx_layer < pcsm_maxLayersPerPass; idx_layer++)
                {
                    // Bottom left:
                    (*l_vertexData).x = l_charDrawBounds.l;
                    (*l_vertexData).y = l_charDrawBounds.b;
                    (*l_vertexData).u = l_fontChar.m_texBounds.l;
                    (*l_vertexData).v = l_fontChar.m_texBounds.b;
                    (*l_vertexData).index = idx_layer;
                    l_vertexData++;

                    // Bottom right:
                    (*l_vertexData).x = l_charDrawBounds.r;
                    (*l_vertexData).y = l_charDrawBounds.b;
                    (*l_vertexData).u = l_fontChar.m_texBounds.r;
                    (*l_vertexData).v = l_fontChar.m_texBounds.b;
                    (*l_vertexData).index = idx_layer;
                    l_vertexData++;

                    // Top left:
                    (*l_vertexData).x = l_charDrawBounds.l;
                    (*l_vertexData).y = l_charDrawBounds.t;
                    (*l_vertexData).u = l_fontChar.m_texBounds.l;
                    (*l_vertexData).v = l_fontChar.m_texBounds.t;
                    (*l_vertexData).index = idx_layer;
                    l_vertexData++;

                    // Top right:
                    (*l_vertexData).x = l_charDrawBounds.r;
                    (*l_vertexData).y = l_charDrawBounds.t;
                    (*l_vertexData).u = l_fontChar.m_texBounds.r;
                    (*l_vertexData).v = l_fontChar.m_texBounds.t;
                    (*l_vertexData).index = idx_layer;
                    l_vertexData++;
                }

                //
                p_drawBufferUsingProfile(in_profile,
                                         in_scalable_font,
                                         in_position,
                                         in_size,
                                         2 * pcsm_maxLayersPerPass); // Char is just 2 prims * layers

                return atl::point2f(in_position.x + l_fontChar.m_advance * in_size, in_position.y);
            }
        }
        return in_position;
    }
}
