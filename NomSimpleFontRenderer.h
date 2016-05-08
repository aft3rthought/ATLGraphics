

#pragma once

#include "ATLUtil/math2d.h"
#include "ATF/ATLGLResource.h"

#include <string>

class NomFiles;
class NomFont;
class NomSimpleFontProfile;
class NomRendererState;

class NomSimpleFontRenderer
{
private:
    NomRendererState & pm_rendererState;
    
    // Constants
    // Uniform index.
    enum
    {
        UNIFORM_SCREEN_DIM,
        UNIFORM_TRANSLATION,
        UNIFORM_EDGES,
        UNIFORM_COLORS,
        UNIFORM_ANTIALIASING_RADIUS,
        UNIFORM_SAMPLER_DISTANCE_FIELD,
        NUM_UNIFORMS
    };
    int pm_shaderUniforms[NUM_UNIFORMS];
    
    enum
    {
        ATTRIBUTE_VERT_POSITION,
        ATTRIBUTE_VERT_TEX_COORD,
        ATTRIBUTE_VERT_INDEX,
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
        GLfloat index;
    };
    Vertex pm_vertexData[pcsm_maxVertsPerPass];
    GLushort pm_indexData[pcsm_maxIndicesPerPass];
    ATLGLProgram pm_glProgram;
    ATLGLBuffer pm_glIndexBuffer;
    ATLGLVertexArray pm_glVertexArray[pcsm_numBuffers];
    ATLGLBuffer pm_glVertexBuffer[pcsm_numBuffers];
    int pm_currentVertexBuffer;
    
    void p_drawBufferUsingProfile(const NomSimpleFontProfile & in_profile,
                                  const NomFont & in_font,
                                  const atl::point2f & in_position,
                                  float in_scale,
                                  unsigned int in_numPrims);
    
public:
    NomSimpleFontRenderer(const NomFiles & in_files,
                          NomRendererState & in_rendererState);
    ~NomSimpleFontRenderer();
    
    void render(const NomSimpleFontProfile & in_profile,
                const NomFont & l_font,
                const atl::box2f & in_drawArea,
                atl::anchoring in_textAnchoring,
                const std::string & in_stringToDraw,
                int in_numLines = 1);
    
    atl::point2f drawCharacter(const NomSimpleFontProfile & in_profile,
                               const NomFont & l_font,
                               const char in_character,
                               const atl::point2f & in_position,
                               const float in_size);
};