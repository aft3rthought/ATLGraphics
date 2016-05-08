

#pragma once

#include "ATLUtil/color.h"
#include "ATLUtil/math2d_fwd.h"
#include "ATF/RenderUtil.h"
#include "ATF/ATLGLResource.h"

#include <vector>

class NomFiles;
class NomRendererState;

//#warning TODO: Use this !
class NomFlatColorGeoBuffers
{
    ATLGLVertexArray m_vbo;
    ATLGLBuffer m_vertexBuffer;
    ATLGLBuffer m_indexBuffer;
    int m_numTriangles;
    bool m_isDynamic;
};

class NomFlatColorGeoRenderer
{
private:
    // Constants
    // Uniform index.
    enum
    {
        UNIFORM_SCREEN_DIM,
        UNIFORM_COLOR,
        UNIFORM_SCALE,
        UNIFORM_OFFSET,
        NUM_UNIFORMS
    };
    int pm_shaderUniforms[NUM_UNIFORMS];
    
    enum
    {
        ATTRIBUTE_VERT_POSITION,
        ATTRIBUTE_VERT_COLOR,
        NUM_ATTRIBUTES
    };
    
    ATLGLProgram pm_glProgram;
    NomRendererState & pm_rendererState;
    
public:
    struct Tri
    {
        unsigned int a, b, c;
        
        Tri() {};
        
        Tri(unsigned int in_a, unsigned int in_b, unsigned int in_c) :
        a(in_a),
        b(in_b),
        c(in_c)
        {}
    };
    
    struct Vertex
    {
        float x, y;
        atl::color_premul color;
        
        Vertex() : color(1.f, 1.f, 1.f, 1.f) {};
        
        Vertex(float in_x, float in_y, atl::color_premul in_color) :
        x(in_x),
        y(in_y),
        color(in_color)
        {}
    };
    
    
    NomFlatColorGeoRenderer(const NomFiles & in_files, NomRendererState & in_rendererState);
    ~NomFlatColorGeoRenderer();

    static void prepareBuffers(ATLGLVertexArray & ref_vertexArray,
                               ATLGLBuffer & ref_vertexBuffer,
                               ATLGLBuffer & ref_indexBuffer,
                               const std::vector<Vertex> & in_vertices,
                               const std::vector<Tri> & in_triangles,
                               bool in_useStaticBuffers = true);
    
    void render(const ATLGLVertexArray & in_vbo,
                const ATLGLBuffer & in_vertexBuffer,
                int in_numTriangles,
                const atl::color & in_color);
    
    void render(const ATLGLVertexArray & in_vbo,
                const ATLGLBuffer & in_vertexBuffer,
                int in_numTriangles,
                const atl::color & in_color,
                const atl::size2f & in_scale,
                const atl::point2f & in_offset);
    
    void renderVerts(const ATLGLVertexArray & in_vbo,
                     const ATLGLBuffer & in_vertexBuffer,
                     int in_numTriangles,
                     const std::vector<Vertex> & in_verts);
    
    void renderVerts(const ATLGLVertexArray & in_vbo,
                     const ATLGLBuffer & in_vertexBuffer,
                     int in_numTriangles,
                     const std::vector<Vertex> & in_verts,
                     const atl::color & in_color,
                     const atl::size2f & in_scale,
                     const atl::point2f & in_offset);
};