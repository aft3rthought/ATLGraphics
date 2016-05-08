

#pragma once

#include "ATLUtil/color.h"
#include "ATLUtil/math3d.h"
#include "ATF/RenderUtil.h"
#include "ATF/ATLGLResource.h"

#include <vector>

class NomFiles;
class NomRendererState;

//#warning TODO: Use this !
class ATFDrawColorGeoBuffer
{
    ATLGLVertexArray m_vbo;
    ATLGLBuffer m_vertexBuffer;
    ATLGLBuffer m_indexBuffer;
    int m_numTriangles;
    bool m_isDynamic;
};

class ATFDrawColorGeo
{
private:
    // Constants
    // Uniform index.
    enum
    {
        UNIFORM_SCREEN_DIM,
        UNIFORM_MODELVIEW_MATRIX,
        UNIFORM_PROJECTION_MATRIX,
        NUM_UNIFORMS
    };
    int pm_shaderUniforms[NUM_UNIFORMS];
    
    enum
    {
        ATTRIBUTE_VERT_POSITION,
        ATTRIBUTE_VERT_COLOR,
        NUM_ATTRIBUTES
    };
    
    ATLGLProgram glProgram;
    NomRendererState & rendererState;
    
public:
    struct Tri
    {
        unsigned int a, b, c;
        
        Tri(unsigned int inA, unsigned int inB, unsigned int inC) :
        a(inA),
        b(inB),
        c(inC)
        {}
    };
    
    struct Vertex
    {
        atl::point3f pos;
        atl::color_premul color;
        
        Vertex(const atl::point3f & inPos, const atl::color_premul & inColor) :
        pos(inPos),
        color(inColor)
        {}
    };
    
    
    ATFDrawColorGeo(const NomFiles & in_files, NomRendererState & in_rendererState);
    ~ATFDrawColorGeo();

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
    
    void renderVerts(const ATLGLVertexArray & in_vbo,
                     const ATLGLBuffer & in_vertexBuffer,
                     int in_numTriangles,
                     const std::vector<Vertex> & in_verts);
};