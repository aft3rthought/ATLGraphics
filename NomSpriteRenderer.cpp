

#include "./NomSpriteRenderer.h"

#include "ATLUtil/array_util.h"
#include "ATLUtil/debug_break.h"
#include "ATF/RenderUtil.h"
#include "ATF/NomSpriteFrame.h"
#include "ATF/NomRendererState.h"

NomSpriteRenderer::NomSpriteRenderer(const NomFiles & in_files, NomRendererState & in_rendererState) :
    pm_colorModulation(atl::color_gray(1.f, 1.f)),
    pm_currentColorModulation(atl::color_gray(1.f, 1.f)),
    pm_currentColorLerp(atl::color_gray(0.f, 0.f)),
    pm_colorLerp(0.f, 0.f, 0.f, 0.f),
    pm_rendererState(in_rendererState),
    pm_currentTexture(0),
    pm_currentSpriteCorners(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
    pm_currentSpriteTexBounds(std::numeric_limits<float>::max(),
                              std::numeric_limits<float>::max(),
                              std::numeric_limits<float>::max(),
                              std::numeric_limits<float>::max())
{
    struct l_SpriteVert
    {
        float c1Component;
        float c2Component;
        float c3Component;
        float c4Component;
    };
    
    l_SpriteVert cl_spriteVerts[4] =
    {
        {1.f, 0.f, 0.f, 0.f},   // c1, l, t    // 0
        {0.f, 0.f, 0.f, 1.f},   // c4, l, b    // 3
        {0.f, 1.f, 0.f, 0.f},   // c2, r, t    // 1
        {0.f, 0.f, 1.f, 0.f},   // c3, r, b    // 2
    };
    
    pm_vertexArray.alloc();
    glBindVertexArray_NOM(pm_vertexArray);
    
    pm_vertexBuffer.alloc();
    glBindBuffer(GL_ARRAY_BUFFER, pm_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(l_SpriteVert) * atl::c_array_len(cl_spriteVerts), cl_spriteVerts, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(ATTRIBUTE_VERT_INDEXES);
    glVertexAttribPointer(ATTRIBUTE_VERT_INDEXES, 4, GL_FLOAT, GL_FALSE, sizeof(l_SpriteVert), 0);
    
    glBindVertexArray_NOM(0);
    
    GLuint vertShader, fragShader;
    
    // Create shader program.
    pm_program.alloc();
    
    // Create and compile vertex shader.
    if (!RenderUtil::compileShader(in_files, &vertShader, GL_VERTEX_SHADER, "Sprite"))
    {
        SGDebugBreak("Failed to compile vertex shader\n");
        return;
    }
    
    // Create and compile fragment shader.
    if (!RenderUtil::compileShader(in_files, &fragShader, GL_FRAGMENT_SHADER, "Sprite"))
    {
        SGDebugBreak("Failed to compile fragment shader\n");
        return;
    }
    
    // Attach vertex shader to program.
    glAttachShader(pm_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(pm_program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(pm_program, ATTRIBUTE_VERT_INDEXES, "vertComponents");
    
    // Link program.
    if (!RenderUtil::linkProgram(pm_program))
    {
        printf("Failed to link program: %i", (GLuint)pm_program);
        
        if (vertShader)
        {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        
        if (fragShader)
        {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        
        pm_program.free();

        return;
    }
    
    // Get uniform locations.
    pm_shaderUniforms[UNIFORM_SPRITE_CORNERS]     = glGetUniformLocation(pm_program, "spriteCorners");
    pm_shaderUniforms[UNIFORM_SPRITE_TEXCOORDS]   = glGetUniformLocation(pm_program, "spriteTexCoords");
    pm_shaderUniforms[UNIFORM_SCREEN_DIM]         = glGetUniformLocation(pm_program, "u_screenBounds");
    pm_shaderUniforms[UNIFORM_COLOR_MODULATION]   = glGetUniformLocation(pm_program, "colorModulation");
    pm_shaderUniforms[UNIFORM_COLOR_LERP]         = glGetUniformLocation(pm_program, "colorLerp");
    
    // Release vertex and fragment shaders.
    if (vertShader)
    {
        glDetachShader(pm_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader)
    {
        glDetachShader(pm_program, fragShader);
        glDeleteShader(fragShader);
    }
}

NomSpriteRenderer::~NomSpriteRenderer()
{
    pm_vertexBuffer.free();
    pm_vertexArray.free();
    pm_program.free();
}

NomSpriteOutputPos NomSpriteRenderer::transform(const NomSpriteFrame & in_spriteFrame,
                                                const atl::point2f & in_spritePosition,
                                                const atl::size2f & in_spriteScale,
                                                float in_spriteRotation)
{
    // Apply scale:
    NomSpriteOutputPos l_corners(in_spriteFrame.m_area.l * in_spriteScale.w, in_spriteFrame.m_area.t * in_spriteScale.h,
                                 in_spriteFrame.m_area.r * in_spriteScale.w, in_spriteFrame.m_area.t * in_spriteScale.h,
                                 in_spriteFrame.m_area.r * in_spriteScale.w, in_spriteFrame.m_area.b * in_spriteScale.h,
                                 in_spriteFrame.m_area.l * in_spriteScale.w, in_spriteFrame.m_area.b * in_spriteScale.h);
    
    
    // Apply rotation:
    float l_sin = sinf(in_spriteRotation);
    float l_cos = cosf(in_spriteRotation);
    
    auto l_rotate = [](atl::point2f & l_pt, float l_sin, float l_cos) {
        l_pt = atl::point2f(l_pt.x * l_cos - l_pt.y * l_sin, l_pt.x * l_sin + l_pt.y * l_cos);
    };
    
    l_rotate(l_corners.c1, l_sin, l_cos);
    l_rotate(l_corners.c2, l_sin, l_cos);
    l_rotate(l_corners.c3, l_sin, l_cos);
    l_rotate(l_corners.c4, l_sin, l_cos);
    
    // Apply translation:
    l_corners.c1 += in_spritePosition;
    l_corners.c2 += in_spritePosition;
    l_corners.c3 += in_spritePosition;
    l_corners.c4 += in_spritePosition;
    
    return l_corners;
}

void NomSpriteRenderer::renderSprite(const NomSpriteFrame & in_spriteFrame,
                                     const atl::point2f & in_spritePosition,
                                     const atl::size2f & in_spriteScale,
                                     float in_spriteRotation)
{
    renderSprite(in_spriteFrame, transform(in_spriteFrame, in_spritePosition, in_spriteScale, in_spriteRotation));
}

void NomSpriteRenderer::renderSprite(const NomSpriteFrame & in_spriteFrame,
                                     const atl::point2f & in_spritePosition)
{
    atl::box2f l_area = in_spriteFrame.m_area;
    l_area += in_spritePosition;
    
    renderSprite(in_spriteFrame, l_area);
}

void NomSpriteRenderer::renderSprite(const NomSpriteFrame & in_spriteFrame,
                  const atl::point2f & in_spritePosition,
                  const atl::size2f & in_spriteScale)
{
    atl::box2f l_area = in_spriteFrame.m_area;
    l_area += in_spritePosition;
    l_area.x.scale(in_spriteScale.w);
    l_area.y.scale(in_spriteScale.h);
    
    renderSprite(in_spriteFrame, l_area);
}

void NomSpriteRenderer::renderSprite(const NomSpriteFrame & in_spriteFrame,
                                     const NomSpriteOutputPos & in_corners)
{
    renderSprite(in_spriteFrame, in_corners, in_spriteFrame.m_imgTexCoords);
}

void NomSpriteRenderer::renderSprite(const NomSpriteFrame & in_spriteFrame,
                                     const NomSpriteOutputPos & in_corners,
                                     const atl::box2f & in_texCoords)
{
    //
    // -1, 1                  1, 1
    //
    //      * -------------- *
    //      |                |
    //      |                |
    //      |                |
    //      |                |
    //      * -------------- *
    //
    // -1, -1                 1, -1
    //
    
    
    if(pm_rendererState.setAsCurrentRenderer(this))
    {
        glBindVertexArray_NOM(pm_vertexArray);
        
        glUseProgram(pm_program);
        
        pm_currentScreenBounds.t =
        pm_currentScreenBounds.r =
        pm_currentScreenBounds.b =
        pm_currentScreenBounds.l =
        pm_currentColorModulation.r =
        pm_currentColorModulation.g =
        pm_currentColorModulation.b =
        pm_currentColorModulation.a =
        pm_currentColorLerp.r =
        pm_currentColorLerp.g =
        pm_currentColorLerp.b =
        pm_currentColorLerp.a =
        pm_currentSpriteCorners.c1.x =
        pm_currentSpriteCorners.c1.y =
        pm_currentSpriteCorners.c2.x =
        pm_currentSpriteCorners.c2.y =
        pm_currentSpriteCorners.c3.x =
        pm_currentSpriteCorners.c3.y =
        pm_currentSpriteCorners.c4.x =
        pm_currentSpriteCorners.c4.y =
        pm_currentSpriteTexBounds.t =
        pm_currentSpriteTexBounds.r =
        pm_currentSpriteTexBounds.b =
        pm_currentSpriteTexBounds.l = std::numeric_limits<float>::max();
        
        pm_currentTexture = std::numeric_limits<decltype(pm_currentTexture)>::max();
    }
    
    if(pm_currentScreenBounds != pm_rendererState.m_currentBounds)
    {
        glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                    pm_rendererState.m_currentBounds.l,
                    pm_rendererState.m_currentBounds.b,
                    pm_rendererState.m_currentBounds.width(),
                    pm_rendererState.m_currentBounds.height());
        
        pm_currentScreenBounds = pm_rendererState.m_currentBounds;
    }
    
    if(pm_currentTexture != in_spriteFrame.m_textureSheetGlHandle)
    {
        glBindTexture(GL_TEXTURE_2D, in_spriteFrame.m_textureSheetGlHandle);
        pm_currentTexture = in_spriteFrame.m_textureSheetGlHandle;
    }
    
    if(pm_currentSpriteCorners != in_corners)
    {
        float l_data[] = {  in_corners.c1.x, in_corners.c1.y,
                            in_corners.c2.x, in_corners.c2.y,
                            in_corners.c3.x, in_corners.c3.y,
                            in_corners.c4.x, in_corners.c4.y };
        
        glUniform2fv(pm_shaderUniforms[UNIFORM_SPRITE_CORNERS], 4, l_data);
        
        pm_currentSpriteCorners = in_corners;
    }
    
    if(pm_currentSpriteTexBounds != in_texCoords)
    {
        float l_data[] = {  in_texCoords.l, in_texCoords.t,
                            in_texCoords.r, in_texCoords.t,
                            in_texCoords.r, in_texCoords.b,
                            in_texCoords.l, in_texCoords.b };
        
        glUniform2fv(pm_shaderUniforms[UNIFORM_SPRITE_TEXCOORDS], 4, l_data);
        
        pm_currentSpriteTexBounds = in_texCoords;
    }
    
    if(pm_currentColorModulation != pm_colorModulation)
    {
        glUniform4f(pm_shaderUniforms[UNIFORM_COLOR_MODULATION], pm_colorModulation.r, pm_colorModulation.g, pm_colorModulation.b, pm_colorModulation.a );
        pm_currentColorModulation = pm_colorModulation;
    }
    
    if(pm_currentColorLerp != pm_colorLerp)
    {
        glUniform4f(pm_shaderUniforms[UNIFORM_COLOR_LERP], pm_colorLerp.r, pm_colorLerp.g, pm_colorLerp.b, pm_colorLerp.a );
        pm_currentColorLerp = pm_colorLerp;
    }
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void NomSpriteRenderer::setColorModulation(const atl::color_premul & in_color)
{
    pm_colorModulation = in_color;
}

void NomSpriteRenderer::setColorLerp(const atl::color & in_color)
{
    pm_colorLerp = in_color;
}

void NomSpriteRenderer::clearColorModulation()
{
    pm_colorModulation.r =
    pm_colorModulation.g =
    pm_colorModulation.b =
    pm_colorModulation.a = 1.f;
}

void NomSpriteRenderer::clearColorLerp()
{
    pm_colorLerp.r =
    pm_colorLerp.g =
    pm_colorLerp.b =
    pm_colorLerp.a = 0.f;
}
