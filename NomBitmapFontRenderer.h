

#pragma once

#include "ATLUtil/math2d.h"
#include "ATLUtil/color.h"
#include "ATF/ATLGLResource.h"

#include <string>

class NomFiles;
class NomBitmapFont;
class NomRendererState;

class NomBitmapFontRenderer
{
private:
    NomRendererState & pm_rendererState;
    
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
    ATLGLProgram pm_glProgram;
    ATLGLBuffer pm_glIndexBuffer;
    ATLGLVertexArray pm_glVertexArray[pcsm_numBuffers];
    ATLGLBuffer pm_glVertexBuffer[pcsm_numBuffers];
    int pm_currentVertexBuffer;
    
    void p_drawBufferUsingProfile(const NomBitmapFont & in_font,
                                  const atl::point2f & in_position,
                                  unsigned int in_numPrims,
                                  const atl::color & in_color);
    
public:
    NomBitmapFontRenderer(const NomFiles & in_files,
                          NomRendererState & in_rendererState);
    ~NomBitmapFontRenderer();
    
    void render(const NomBitmapFont & l_font,
                const atl::box2f & in_drawArea,
                atl::anchoring in_textAnchoring,
                const std::string & in_stringToDraw,
                const atl::color & in_color,
                int in_numLines = 1);
    
    atl::point2f drawCharacter(const NomBitmapFont & l_font,
                               const char in_character,
                               const atl::point2f & in_position,
                               const atl::color & in_color);
};