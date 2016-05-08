

#include "./ATFDrawColorGeo.h"

#include "ATLUtil/debug_break.h"
#include "ATF/NomRendererState.h"

ATFDrawColorGeo::ATFDrawColorGeo(const NomFiles & in_files,
                                 NomRendererState & in_rendererState) :
    rendererState(in_rendererState)
{    
    GLuint l_vertShader, l_fragShader;
    
    // Create shader program.
    glProgram.alloc();
    
    // Create and compile vertex shader.
    if (!RenderUtil::compileShader(in_files, &l_vertShader, GL_VERTEX_SHADER, "FlatColorGeo"))
    {
        SGDebugBreak("Failed to compile vertex shader\n");
        return;
    }
    
    // Create and compile fragment shader.
    if (!RenderUtil::compileShader(in_files, &l_fragShader, GL_FRAGMENT_SHADER, "FlatColorGeo"))
    {
        SGDebugBreak("Failed to compile fragment shader\n");
        return;
    }
    
    // Attach vertex shader to program.
    glAttachShader(glProgram, l_vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(glProgram, l_fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(glProgram, ATTRIBUTE_VERT_POSITION,  "v_vertPosition");
    glBindAttribLocation(glProgram, ATTRIBUTE_VERT_COLOR,     "v_vertColor");
    
    // Link program.
    if (!RenderUtil::linkProgram(glProgram))
    {
        printf("Failed to link program: %i", (GLuint)glProgram);
        
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
        
        glProgram.free();
        
        return;
    }
    
    // Get uniform locations.
    pm_shaderUniforms[UNIFORM_SCREEN_DIM]   = glGetUniformLocation(glProgram, "u_screenBounds");
    pm_shaderUniforms[UNIFORM_MODELVIEW_MATRIX]   = glGetUniformLocation(glProgram, "u_modelView");
    pm_shaderUniforms[UNIFORM_PROJECTION_MATRIX]   = glGetUniformLocation(glProgram, "u_projection");
    
    // Release vertex and fragment shaders.
    if (l_vertShader)
    {
        glDetachShader(glProgram, l_vertShader);
        glDeleteShader(l_vertShader);
    }
    if (l_fragShader)
    {
        glDetachShader(glProgram, l_fragShader);
        glDeleteShader(l_fragShader);
    }
}

ATFDrawColorGeo::~ATFDrawColorGeo()
{
    glProgram.free();
}

void ATFDrawColorGeo::prepareBuffers(ATLGLVertexArray & ref_vertexArray,
                                     ATLGLBuffer & ref_vertexBuffer,
                                     ATLGLBuffer & ref_indexBuffer,
                                     const std::vector<Vertex> & in_vertices,
                                     const std::vector<Tri> & in_triangles,
                                     bool in_useStaticBuffers)
{
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
    
    SGDebugBreakIf(ref_vertexArray.isInvalid(), "Allocate this ahead of time!");
    SGDebugBreakIf(ref_vertexBuffer.isInvalid(), "Allocate this ahead of time!");
    SGDebugBreakIf(ref_indexBuffer.isInvalid(), "Allocate this ahead of time!");
    
    glBindVertexArray_NOM(ref_vertexArray);
    
    glBindBuffer(GL_ARRAY_BUFFER, ref_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * in_vertices.size(), in_vertices.data(), in_useStaticBuffers ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ref_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Tri) * in_triangles.size(), in_triangles.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(ATTRIBUTE_VERT_POSITION);
    glVertexAttribPointer(ATTRIBUTE_VERT_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(ATFDrawColorGeo::Vertex), 0);
    
    glEnableVertexAttribArray(ATTRIBUTE_VERT_COLOR);
    glVertexAttribPointer(ATTRIBUTE_VERT_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(ATFDrawColorGeo::Vertex), BUFFER_OFFSET(2 * sizeof(float)));
#undef BUFFER_OFFSET
    
    glBindVertexArray_NOM(0);
}

void ATFDrawColorGeo::render(const ATLGLVertexArray & in_vbo,
                             const ATLGLBuffer & in_vertexBuffer,
                             int in_numTriangles,
                             const atl::color & in_color)
{
    /*
    if(pm_rendererState.setAsCurrentRenderer(this))
    {
        glUseProgram(pm_glProgram);
        
        glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                    pm_rendererState.m_currentBounds.l,
                    pm_rendererState.m_currentBounds.b,
                    pm_rendererState.m_currentBounds.width(),
                    pm_rendererState.m_currentBounds.height());
    }
    
    glUniform4f(pm_shaderUniforms[UNIFORM_COLOR], in_color.r, in_color.g, in_color.b, in_color.a);
    glUniform2f(pm_shaderUniforms[UNIFORM_SCALE], in_scale.w, in_scale.h);
    glUniform2f(pm_shaderUniforms[UNIFORM_OFFSET], in_offset.x, in_offset.y);
    
    glBindVertexArray_NOM(in_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, in_vertexBuffer);
    
    glDrawElements(GL_TRIANGLES, in_numTriangles * 3, GL_UNSIGNED_INT, (void*)0);
     */
}

void ATFDrawColorGeo::renderVerts(const ATLGLVertexArray & in_vbo,
                                  const ATLGLBuffer & in_vertexBuffer,
                                  int in_numTriangles,
                                  const std::vector<Vertex> & in_vertices)
{
    /*
    if(pm_rendererState.setAsCurrentRenderer(this))
    {
        glUseProgram(pm_glProgram);
        
        glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                    pm_rendererState.m_currentBounds.l,
                    pm_rendererState.m_currentBounds.b,
                    pm_rendererState.m_currentBounds.width(),
                    pm_rendererState.m_currentBounds.height());
    }
    
    glUniform4f(pm_shaderUniforms[UNIFORM_COLOR], 1.f, 1.f, 1.f, 1.f);
    glUniform2f(pm_shaderUniforms[UNIFORM_SCALE], 1.f, 1.f);
    glUniform2f(pm_shaderUniforms[UNIFORM_OFFSET], 0.f, 0.f);
    
    glBindVertexArray_NOM(in_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, in_vertexBuffer);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * in_vertices.size(), in_vertices.data());
    
    glDrawElements(GL_TRIANGLES, in_numTriangles * 3, GL_UNSIGNED_INT, (void*)0);
     */
}
