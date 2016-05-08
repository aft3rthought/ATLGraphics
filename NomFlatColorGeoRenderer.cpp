

#include "./NomFlatColorGeoRenderer.h"

#include "ATLUtil/debug_break.h"
#include "ATF/NomRendererState.h"

NomFlatColorGeoRenderer::NomFlatColorGeoRenderer(const NomFiles & in_files,
                                                 NomRendererState & in_rendererState) :
    pm_rendererState(in_rendererState)
{    
    GLuint l_vertShader, l_fragShader;
    
    // Create shader program.
    pm_glProgram.alloc();
    
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
    glAttachShader(pm_glProgram, l_vertShader);
    atf::check_gl_errors();
    
    // Attach fragment shader to program.
    glAttachShader(pm_glProgram, l_fragShader);
    atf::check_gl_errors();
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(pm_glProgram, ATTRIBUTE_VERT_POSITION,  "v_vertPosition");
    atf::check_gl_errors();
    glBindAttribLocation(pm_glProgram, ATTRIBUTE_VERT_COLOR,     "v_vertColor");
    atf::check_gl_errors();
    
    // Link program.
    if (!RenderUtil::linkProgram(pm_glProgram))
    {
        printf("Failed to link program: %i", (GLuint)pm_glProgram);
        
        if (l_vertShader)
        {
            glDeleteShader(l_vertShader);
            atf::check_gl_errors();
            l_vertShader = 0;
        }
        if (l_fragShader)
        {
            glDeleteShader(l_fragShader);
            atf::check_gl_errors();
            l_fragShader = 0;
        }
        
        pm_glProgram.free();
        
        return;
    }
    
    // Get uniform locations.
    pm_shaderUniforms[UNIFORM_SCREEN_DIM]   = glGetUniformLocation(pm_glProgram, "u_screenBounds");
    pm_shaderUniforms[UNIFORM_COLOR]   = glGetUniformLocation(pm_glProgram, "u_color");
    pm_shaderUniforms[UNIFORM_SCALE]   = glGetUniformLocation(pm_glProgram, "u_scale");
    pm_shaderUniforms[UNIFORM_OFFSET]   = glGetUniformLocation(pm_glProgram, "u_offset");
    
    // Release vertex and fragment shaders.
    if (l_vertShader)
    {
        glDetachShader(pm_glProgram, l_vertShader);
        atf::check_gl_errors();
        glDeleteShader(l_vertShader);
        atf::check_gl_errors();
    }
    if (l_fragShader)
    {
        glDetachShader(pm_glProgram, l_fragShader);
        atf::check_gl_errors();
        glDeleteShader(l_fragShader);
        atf::check_gl_errors();
    }
}

NomFlatColorGeoRenderer::~NomFlatColorGeoRenderer()
{
    pm_glProgram.free();
}

void NomFlatColorGeoRenderer::prepareBuffers(ATLGLVertexArray & ref_vertexArray,
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
    atf::check_gl_errors();
    
    glBindBuffer(GL_ARRAY_BUFFER, ref_vertexBuffer);
    atf::check_gl_errors();
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * in_vertices.size(), in_vertices.data(), in_useStaticBuffers ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    atf::check_gl_errors();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ref_indexBuffer);
    atf::check_gl_errors();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Tri) * in_triangles.size(), in_triangles.data(), GL_STATIC_DRAW);
    atf::check_gl_errors();
    
    glEnableVertexAttribArray(ATTRIBUTE_VERT_POSITION);
    atf::check_gl_errors();
    glVertexAttribPointer(ATTRIBUTE_VERT_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(NomFlatColorGeoRenderer::Vertex), 0);
    atf::check_gl_errors();
    
    glEnableVertexAttribArray(ATTRIBUTE_VERT_COLOR);
    atf::check_gl_errors();
    glVertexAttribPointer(ATTRIBUTE_VERT_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(NomFlatColorGeoRenderer::Vertex), BUFFER_OFFSET(2 * sizeof(float)));
    atf::check_gl_errors();
#undef BUFFER_OFFSET
    
    glBindVertexArray_NOM(0);
    atf::check_gl_errors();
}

void NomFlatColorGeoRenderer::render(const ATLGLVertexArray & in_vbo,
                                     const ATLGLBuffer & in_vertexBuffer,
                                     int in_numTriangles,
                                     const atl::color & in_color)
{
    render(in_vbo,
           in_vertexBuffer,
           in_numTriangles,
           in_color,
           atl::size2f::Identity,
           atl::point2f(0.f, 0.f));
}

void NomFlatColorGeoRenderer::render(const ATLGLVertexArray & in_vbo,
                                     const ATLGLBuffer & in_vertexBuffer,
                                     int in_numTriangles,
                                     const atl::color & in_color,
                                     const atl::size2f & in_scale,
                                     const atl::point2f & in_offset)
{
    if(pm_rendererState.setAsCurrentRenderer(this))
    {
        glUseProgram(pm_glProgram);
        atf::check_gl_errors();
        
        glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                    pm_rendererState.m_currentBounds.l,
                    pm_rendererState.m_currentBounds.b,
                    pm_rendererState.m_currentBounds.width(),
                    pm_rendererState.m_currentBounds.height());
        atf::check_gl_errors();
    }
    
    glUniform4f(pm_shaderUniforms[UNIFORM_COLOR], in_color.r, in_color.g, in_color.b, in_color.a);
    atf::check_gl_errors();
    glUniform2f(pm_shaderUniforms[UNIFORM_SCALE], in_scale.w, in_scale.h);
    atf::check_gl_errors();
    glUniform2f(pm_shaderUniforms[UNIFORM_OFFSET], in_offset.x, in_offset.y);
    atf::check_gl_errors();
    
    glBindVertexArray_NOM(in_vbo);
    atf::check_gl_errors();
    glBindBuffer(GL_ARRAY_BUFFER, in_vertexBuffer);
    atf::check_gl_errors();
    
    glDrawElements(GL_TRIANGLES, in_numTriangles * 3, GL_UNSIGNED_INT, (void*)0);
    atf::check_gl_errors();
}

void NomFlatColorGeoRenderer::renderVerts(const ATLGLVertexArray & in_vbo,
                                          const ATLGLBuffer & in_vertexBuffer,
                                          int in_numTriangles,
                                          const std::vector<Vertex> & in_vertices)
{
    renderVerts(in_vbo, in_vertexBuffer, in_numTriangles, in_vertices, atl::color_white, atl::size2f::Identity, atl::point2f::Zero);
}

void NomFlatColorGeoRenderer::renderVerts(const ATLGLVertexArray & in_vbo,
                                          const ATLGLBuffer & in_vertexBuffer,
                                          int in_numTriangles,
                                          const std::vector<Vertex> & in_vertices,
                                          const atl::color & in_color,
                                          const atl::size2f & in_scale,
                                          const atl::point2f & in_offset)
{
    if(pm_rendererState.setAsCurrentRenderer(this))
    {
        glUseProgram(pm_glProgram);
        atf::check_gl_errors();
        
        glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                    pm_rendererState.m_currentBounds.l,
                    pm_rendererState.m_currentBounds.b,
                    pm_rendererState.m_currentBounds.width(),
                    pm_rendererState.m_currentBounds.height());
        atf::check_gl_errors();
    }
    
    glUniform4f(pm_shaderUniforms[UNIFORM_COLOR], in_color.r, in_color.g, in_color.b, in_color.a);
    atf::check_gl_errors();
    glUniform2f(pm_shaderUniforms[UNIFORM_SCALE], in_scale.w, in_scale.h);
    atf::check_gl_errors();
    glUniform2f(pm_shaderUniforms[UNIFORM_OFFSET], in_offset.x, in_offset.y);
    atf::check_gl_errors();
    
    glBindVertexArray_NOM(in_vbo);
    atf::check_gl_errors();
    glBindBuffer(GL_ARRAY_BUFFER, in_vertexBuffer);
    atf::check_gl_errors();
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * in_vertices.size(), in_vertices.data());
    atf::check_gl_errors();
    
    glDrawElements(GL_TRIANGLES, in_numTriangles * 3, GL_UNSIGNED_INT, (void*)0);
    atf::check_gl_errors();
}
