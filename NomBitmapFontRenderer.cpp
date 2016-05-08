

#include "./NomBitmapFontRenderer.h"

#include "ATLUtil/array_util.h"
#include "ATLUtil/debug_break.h"
#include "ATF/NomFiles.h"
#include "ATF/NomBitmapFont.h"
#include "ATF/RenderUtil.h"
#include "ATF/NomRendererState.h"

NomBitmapFontRenderer::NomBitmapFontRenderer(const NomFiles & in_files,
                                             NomRendererState & in_rendererState) :
    pm_currentVertexBuffer(0),
    pm_rendererState(in_rendererState)
{
    // Some compile time metadata about our vector data:
    const GLsizei l_dynamicDataVertexStride = sizeof(NomBitmapFontRenderer::Vertex);
    
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
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
    
    for(int i = 0; i < pcsm_numBuffers; i++)
    {
        pm_glVertexArray[i].alloc();
        pm_glVertexBuffer[i].alloc();
    }
    pm_glIndexBuffer.alloc();
    
    for(int idx_buffer = 0; idx_buffer < pcsm_numBuffers; idx_buffer++)
    {
        glBindVertexArray_NOM(pm_glVertexArray[idx_buffer]);
        
        glBindBuffer(GL_ARRAY_BUFFER, pm_glVertexBuffer[idx_buffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pm_vertexData), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pm_glIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pm_indexData), &pm_indexData[0], GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(ATTRIBUTE_VERT_POSITION);
        glVertexAttribPointer(ATTRIBUTE_VERT_POSITION, 2, GL_FLOAT, GL_FALSE, l_dynamicDataVertexStride, 0);
        
        glEnableVertexAttribArray(ATTRIBUTE_VERT_TEX_COORD);
        glVertexAttribPointer(ATTRIBUTE_VERT_TEX_COORD, 2, GL_FLOAT, GL_FALSE, l_dynamicDataVertexStride, (const GLvoid*)offsetof(class NomBitmapFontRenderer::Vertex, u));
    }
    
#undef BUFFER_OFFSET
    
    glBindVertexArray_NOM(0);
    
    GLuint l_vertShader, l_fragShader;
    
    // Create shader program.
    pm_glProgram.alloc();
    
    // Create and compile vertex shader.
    if (!RenderUtil::compileShader(in_files, &l_vertShader, GL_VERTEX_SHADER, "BitmapFont"))
    {
        SGDebugBreak("Failed to compile vertex shader\n");
        return;
    }
    
    // Create and compile fragment shader.
    if (!RenderUtil::compileShader(in_files, &l_fragShader, GL_FRAGMENT_SHADER, "BitmapFont"))
    {
        SGDebugBreak("Failed to compile fragment shader\n");
        return;
    }
    
    // Attach vertex shader to program.
    glAttachShader(pm_glProgram, l_vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(pm_glProgram, l_fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(pm_glProgram, ATTRIBUTE_VERT_POSITION,  "v_position");
    glBindAttribLocation(pm_glProgram, ATTRIBUTE_VERT_TEX_COORD, "v_texCoord");
    
    // Link program.
    if (!RenderUtil::linkProgram(pm_glProgram))
    {
        printf("Failed to link program: %i", (GLuint)pm_glProgram);
        
        if (l_vertShader)
        {
            glDeleteShader(l_vertShader);
            l_vertShader = 0;
        }
        if (l_fragShader)
        {
            glDeleteShader(l_fragShader);
            l_fragShader = 0;
        }
        
        pm_glProgram.free();
        
        return;
    }
    
    // Get uniform locations.
    pm_shaderUniforms[UNIFORM_SCREEN_DIM] = glGetUniformLocation(pm_glProgram, "u_screenBounds");
    pm_shaderUniforms[UNIFORM_TRANSLATION] = glGetUniformLocation(pm_glProgram, "u_translation");
    pm_shaderUniforms[UNIFORM_SAMPLER_TEXTURE] = glGetUniformLocation(pm_glProgram, "s_texture");
    pm_shaderUniforms[UNIFORM_FONT_COLOR] = glGetUniformLocation(pm_glProgram, "s_fontColor");
    
    // Release vertex and fragment shaders.
    if (l_vertShader)
    {
        glDetachShader(pm_glProgram, l_vertShader);
        glDeleteShader(l_vertShader);
    }
    if (l_fragShader)
    {
        glDetachShader(pm_glProgram, l_fragShader);
        glDeleteShader(l_fragShader);
    }
}

NomBitmapFontRenderer::~NomBitmapFontRenderer()
{
    pm_glIndexBuffer.free();
    pm_glProgram.free();
    for(int i = 0; i < pcsm_numBuffers; i++)
    {
        pm_glVertexArray[i].free();
        pm_glVertexBuffer[i].free();
    }
}

void NomBitmapFontRenderer::render(const NomBitmapFont & l_font,
                                   const atl::box2f & l_drawArea,
                                   atl::anchoring l_textAnchoring,
                                   const std::string & l_stringToDraw,
                                   const atl::color & in_color,
                                   int in_numLines)
{
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
            
            l_Char(int in_fontElementIdx) :
            m_fontElementIdx(in_fontElementIdx)
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
    
    float l_totalLength = 0.f;
    {
        atl::point2f l_curDrawPos(0.f, 0.f);
        l_whitespaceUnits.emplace_back();
        
        bool l_inTag = false;
        std::string l_tagString;
        const char * l_str = l_stringToDraw.c_str();
        for(int idx = 0; idx < l_stringToDraw.length(); idx++, l_str++)
        {
            if(l_inTag)
            {
                if(*l_str == '>')
                {
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
                    float l_scaledSpaceAdvance = l_font.m_spaceCharacterSize;
                    
                    l_whitespaceUnits.back().m_whitespaceAdvance = l_scaledSpaceAdvance;
                    
                    l_totalLength += l_scaledSpaceAdvance;
                    
                    l_curDrawPos = atl::point2f(0.f, 0.f);
                    
                    if(l_whitespaceUnits.back().m_bounds != atl::box2f::MaxInvertedBounds)
                        l_whitespaceUnits.emplace_back();
                }
                else if(*l_str == '\n')
                {
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
                    for(int idx_fontChar = 0; idx_fontChar < l_font.m_fontChars.size(); idx_fontChar++)
                    {
                        const auto & l_fontChar = l_font.m_fontChars[idx_fontChar];
                        if(l_fontChar.m_char == *l_str)
                        {
                            atl::point2f l_pt1(l_curDrawPos.x + l_fontChar.m_offset.x,
                                               l_curDrawPos.y);
                            
                            atl::point2f l_pt2(l_curDrawPos.x + l_fontChar.m_offset.x + l_fontChar.m_size.w,
                                               l_curDrawPos.y + (float)l_font.m_fontSize);
                            
                            l_whitespaceUnits.back().m_bounds.include(l_pt1);
                            l_whitespaceUnits.back().m_bounds.include(l_pt2);
                            l_whitespaceUnits.back().m_chars.emplace_back(idx_fontChar);
                            
                            float l_advanceAmountForChar = l_fontChar.m_advance;
                            
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
    
    // Now do the layout pass:
    atl::box2f l_drawBounds = atl::box2f::MaxInvertedBounds;
    {
        int l_currentLineIdx = 0;
        
        atl::point2f l_curPos(0.f,0.f);
        for(auto & l_whitespaceUnit : l_whitespaceUnits)
        {
            l_whitespaceUnit.m_line = l_currentLineIdx;
            l_whitespaceUnit.m_drawPos = l_curPos;
            l_whitespaceUnit.m_bounds += l_curPos;
            l_drawBounds.include(l_whitespaceUnit.m_bounds);
            
            l_curPos.x += l_whitespaceUnit.m_advance;
            l_curPos.x += l_whitespaceUnit.m_whitespaceAdvance;
            
            if(l_whitespaceUnit.m_linebreak)
            {
                l_curPos.x = 0.f;
                l_curPos.y -= l_font.m_fontSize * l_font.m_lineHeight;
                l_currentLineIdx++;
            }
        }
        
        {
            float l_yAdjustment = (float)l_currentLineIdx * l_font.m_fontSize * l_font.m_lineHeight;
            
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
    
    // Apply gravity:
    l_drawBounds = l_drawArea.get_sub_bounds_with_size(l_drawBounds.size(), l_textAnchoring);
    
    // Now draw string:
    {
        NomBitmapFontRenderer::Vertex * l_vertexData = pm_vertexData;
        NomBitmapFontRenderer::Vertex * l_end = pm_vertexData + atl::c_array_len(pm_vertexData);
        unsigned int l_numPrims = 0;
        
        for(const auto & l_whitespaceUnit : l_whitespaceUnits)
        {
            atl::point2f l_curDrawPos = l_whitespaceUnit.m_drawPos;
            
            for(const auto & l_layoutChar : l_whitespaceUnit.m_chars)
            {
                const auto & l_fontChar = l_font.m_fontChars[l_layoutChar.m_fontElementIdx];
                
                atl::point2f l_charDrawPos = l_curDrawPos;
                l_charDrawPos += l_fontChar.m_offset;
                
                atl::box2f l_charDrawBounds(l_charDrawPos.y + l_fontChar.m_size.h,
                                               l_charDrawPos.x + l_fontChar.m_size.w,
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
                    l_vertexData++;
                    
                    // Bottom right:
                    (*l_vertexData).x = l_charDrawBounds.r;
                    (*l_vertexData).y = l_charDrawBounds.b;
                    (*l_vertexData).u = l_fontChar.m_texBounds.r;
                    (*l_vertexData).v = l_fontChar.m_texBounds.b;
                    l_vertexData++;
                    
                    // Top left:
                    (*l_vertexData).x = l_charDrawBounds.l;
                    (*l_vertexData).y = l_charDrawBounds.t;
                    (*l_vertexData).u = l_fontChar.m_texBounds.l;
                    (*l_vertexData).v = l_fontChar.m_texBounds.t;
                    l_vertexData++;
                    
                    // Top right:
                    (*l_vertexData).x = l_charDrawBounds.r;
                    (*l_vertexData).y = l_charDrawBounds.t;
                    (*l_vertexData).u = l_fontChar.m_texBounds.r;
                    (*l_vertexData).v = l_fontChar.m_texBounds.t;
                    l_vertexData++;
                }
                
                l_curDrawPos.x += l_fontChar.m_advance;
            }
        }
        
        if(l_vertexData != pm_vertexData)
        {
            p_drawBufferUsingProfile(l_font,
                                     atl::point2f(l_drawBounds.l, l_drawBounds.b),
                                     l_numPrims,
                                     in_color);
        }
    }
}

void NomBitmapFontRenderer::p_drawBufferUsingProfile(const NomBitmapFont & in_font,
                                                     const atl::point2f & in_position,
                                                     unsigned int in_numPrims,
                                                     const atl::color & in_color)
{
    glBindVertexArray_NOM(pm_glVertexArray[pm_currentVertexBuffer]);
    glBindBuffer(GL_ARRAY_BUFFER, pm_glVertexBuffer[pm_currentVertexBuffer]);
    
    // Upload vertex data:
    glBufferSubData(GL_ARRAY_BUFFER, 0, atl::c_array_len(pm_vertexData) * sizeof(NomBitmapFontRenderer::Vertex), pm_vertexData);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in_font.m_texture);
    
    // Shared uniforms:
    glUniform2f(pm_shaderUniforms[UNIFORM_TRANSLATION], in_position.x, in_position.y);
    glUniform1i(pm_shaderUniforms[UNIFORM_SAMPLER_TEXTURE], 0);
    glUniform4f(pm_shaderUniforms[UNIFORM_FONT_COLOR],
                in_color.r,
                in_color.g,
                in_color.b,
                in_color.a);
    
    glDrawElements(GL_TRIANGLES, in_numPrims * 3, GL_UNSIGNED_SHORT, (void*)0);
    
    // Swap buffers:
    pm_currentVertexBuffer = (pm_currentVertexBuffer + 1) % pcsm_numBuffers;
}


atl::point2f NomBitmapFontRenderer::drawCharacter(const NomBitmapFont & l_font,
                                                  const char in_character,
                                                  const atl::point2f & in_position,
                                                  const atl::color & in_color)
{
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
    for(const auto & l_fontChar : l_font.m_fontChars)
    {
        if(l_fontChar.m_char == in_character)
        {
            atl::box2f l_charDrawBounds(l_fontChar.m_offset.y + l_fontChar.m_size.h,
                                           l_fontChar.m_offset.x + l_fontChar.m_size.w,
                                           l_fontChar.m_offset.y,
                                           l_fontChar.m_offset.x);
            
            NomBitmapFontRenderer::Vertex * l_vertexData = pm_vertexData;
            
            // Bottom left:
            (*l_vertexData).x = l_charDrawBounds.l;
            (*l_vertexData).y = l_charDrawBounds.b;
            (*l_vertexData).u = l_fontChar.m_texBounds.l;
            (*l_vertexData).v = l_fontChar.m_texBounds.b;
            l_vertexData++;
            
            // Bottom right:
            (*l_vertexData).x = l_charDrawBounds.r;
            (*l_vertexData).y = l_charDrawBounds.b;
            (*l_vertexData).u = l_fontChar.m_texBounds.r;
            (*l_vertexData).v = l_fontChar.m_texBounds.b;
            l_vertexData++;
            
            // Top left:
            (*l_vertexData).x = l_charDrawBounds.l;
            (*l_vertexData).y = l_charDrawBounds.t;
            (*l_vertexData).u = l_fontChar.m_texBounds.l;
            (*l_vertexData).v = l_fontChar.m_texBounds.t;
            l_vertexData++;
            
            // Top right:
            (*l_vertexData).x = l_charDrawBounds.r;
            (*l_vertexData).y = l_charDrawBounds.t;
            (*l_vertexData).u = l_fontChar.m_texBounds.r;
            (*l_vertexData).v = l_fontChar.m_texBounds.t;
            l_vertexData++;
            
            //
            p_drawBufferUsingProfile(l_font,
                                     in_position,
                                     2, // Char is just 2 prims * layers
                                     in_color);
            
            return atl::point2f(in_position.x + l_fontChar.m_advance, in_position.y);
        }
    }
    return in_position;
}
