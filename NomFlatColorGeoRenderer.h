

#pragma once

#include "ATLUtil/color.h"
#include "ATLUtil/math2d_fwd.h"
#include "ATF/RenderUtil.h"
#include "ATF/NomFiles.h"
#include "ATF/ATLGLResource.h"

#include <vector>

namespace atl_graphics_namespace_config
{
    struct shared_renderer_state;

    enum class flat_color_geometry_renderer_status
    {
        waiting_to_load,
        loading,
        ready,
        failed
    };

    class flat_color_geometry_renderer
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

        shader_program_resource internal_program_gl_handle;
        flat_color_geometry_renderer_status internal_status;
        file_data internal_vertex_shader_file;
        file_data internal_fragment_shader_file;

        shared_renderer_state & internal_shared_renderer_state;

        static void internal_configure_vertex_array(const buffer_resource & in_vertexBuffer, const buffer_resource & in_indexBuffer);

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

        flat_color_geometry_renderer(shared_renderer_state & in_rendererState);
        ~flat_color_geometry_renderer();

        flat_color_geometry_renderer_status status() const { return internal_status; }
        void incremental_load(const application_folder & in_application_folder);

        static void prepareBuffers(const vertex_array_resource & in_vertexArray,
                                   const buffer_resource & in_vertexBuffer,
                                   const buffer_resource & in_indexBuffer,
                                   const std::vector<Vertex> & in_vertices,
                                   const std::vector<Tri> & in_triangles,
                                   const bool in_useStaticBuffers = true);

        void render(const vertex_array_resource & in_vbo,
                    const buffer_resource & in_vertexBuffer,
                    const buffer_resource & in_indexBuffer,
                    const int in_numTriangles,
                    const atl::color & in_color);

        void render(const vertex_array_resource & in_vbo,
                    const buffer_resource & in_vertexBuffer,
                    const buffer_resource & in_indexBuffer,
                    const int in_numTriangles,
                    const atl::color & in_color,
                    const atl::size2f & in_scale,
                    const atl::point2f & in_offset);

        void renderVerts(const vertex_array_resource & in_vbo,
                         const buffer_resource & in_vertexBuffer,
                         const buffer_resource & in_indexBuffer,
                         const int in_numTriangles,
                         const std::vector<Vertex> & in_verts);

        void renderVerts(const vertex_array_resource & in_vbo,
                         const buffer_resource & in_vertexBuffer,
                         const buffer_resource & in_indexBuffer,
                         const int in_numTriangles,
                         const std::vector<Vertex> & in_verts,
                         const atl::color & in_color,
                         const atl::size2f & in_scale,
                         const atl::point2f & in_offset);
    };
}
