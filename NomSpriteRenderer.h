

#pragma once

#include "ATLUtil/math2d.h"
#include "ATLUtil/color.h"

#include "ATF/ATLGLResource.h"
#include "ATF/NomRenderingHeaders.h"

class NomSpriteFrame;
class NomFiles;
class NomRendererState;

class NomSpriteOutputPos
{
public:
    atl::point2f c1, c2, c3, c4;

    NomSpriteOutputPos(atl::box2f in_initBounds) :
    c1(in_initBounds.l, in_initBounds.t),
    c2(in_initBounds.r, in_initBounds.t),
    c3(in_initBounds.r, in_initBounds.b),
    c4(in_initBounds.l, in_initBounds.b)
    {}
    
    NomSpriteOutputPos(const atl::point2f & in_c1,
                       const atl::point2f & in_c2,
                       const atl::point2f & in_c3,
                       const atl::point2f & in_c4) :
    c1(in_c1),
    c2(in_c2),
    c3(in_c3),
    c4(in_c4)
    {}
    
    NomSpriteOutputPos(float c1x, float c1y,
                       float c2x, float c2y,
                       float c3x, float c3y,
                       float c4x, float c4y) :
    c1(c1x, c1y),
    c2(c2x, c2y),
    c3(c3x, c3y),
    c4(c4x, c4y)
    {}
    
    bool operator != (const NomSpriteOutputPos & in_other)
    {
        return  c1 != in_other.c1 ||
                c2 != in_other.c2 ||
                c3 != in_other.c3 ||
                c4 != in_other.c4;
    }
};

class NomSpriteRenderer
{
private:
    NomRendererState & pm_rendererState;
    
    atl::color_premul pm_colorModulation;
    atl::color pm_colorLerp;
    
    GLuint pm_currentTexture;
    NomSpriteOutputPos pm_currentSpriteCorners;
    atl::box2f pm_currentSpriteTexBounds;
    atl::color_premul pm_currentColorModulation;
    atl::color pm_currentColorLerp;
    atl::box2f pm_currentScreenBounds;
    
    ATLGLProgram pm_program;
    ATLGLVertexArray pm_vertexArray;
    ATLGLBuffer pm_vertexBuffer;
    
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
    
public:
    NomSpriteRenderer(const NomFiles & in_files, NomRendererState & in_rendererState);
    ~NomSpriteRenderer();

    NomSpriteOutputPos transform(const NomSpriteFrame & in_spriteFrame,
                                 const atl::point2f & in_spritePosition,
                                 const atl::size2f & in_spriteScale,
                                 float in_spriteRotation);

    void renderSprite(const NomSpriteFrame & in_spriteFrame,
                      const atl::point2f & in_spritePosition);

    void renderSprite(const NomSpriteFrame & in_spriteFrame,
                      const atl::point2f & in_spritePosition,
                      const atl::size2f & in_spriteScale);
    
    void renderSprite(const NomSpriteFrame & in_spriteFrame,
                      const atl::point2f & in_spritePosition,
                      const atl::size2f & in_spriteScale,
                      float in_spriteRotation);

    void renderSprite(const NomSpriteFrame & in_spriteFrame,
                      const NomSpriteOutputPos & in_corners);

    void renderSprite(const NomSpriteFrame & in_spriteFrame,
                      const NomSpriteOutputPos & in_corners,
                      const atl::box2f & in_texCoords);
    
    void setColorModulation(const atl::color_premul & in_color);
    void setColorLerp(const atl::color & in_color);
    
    void clearColorModulation();
    void clearColorLerp();
};

class NomSpriteScopedModulation
{
public:
    NomSpriteScopedModulation(const atl::color & inColor, NomSpriteRenderer & inRenderer) :
    mRenderer(inRenderer)
    {
        mRenderer.setColorModulation(inColor);
    }
    
    ~NomSpriteScopedModulation()
    {
        mRenderer.clearColorModulation();
    }
    
private:
    NomSpriteRenderer & mRenderer;
};

class NomSpriteScopedLerp
{
public:
    NomSpriteScopedLerp(const atl::color & inColor, NomSpriteRenderer & inRenderer) :
    mRenderer(inRenderer)
    {
        mRenderer.setColorLerp(inColor);
    }
    
    ~NomSpriteScopedLerp()
    {
        mRenderer.clearColorLerp();
    }
    
private:
    NomSpriteRenderer & mRenderer;
};
