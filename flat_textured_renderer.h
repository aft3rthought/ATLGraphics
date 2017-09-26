

#pragma once

#include "ATLUtil/math2d.h"
#include "ATLUtil/color.h"

#include "ATF/ATLGLResource.h"
#include "ATF/NomRenderingHeaders.h"
#include "ATF/NomFiles.h"

namespace atl_graphics_namespace_config
{
    struct application_folder;
    struct shared_renderer_state;

    struct flat_textured_vertex
    {
        atl::point2f position;
        atl::point2f texture_coordinates;
        atl::color color_multiply;
        atl::color color_lerp;
    };
    
    enum class flat_textured_renderer_status
    {
        waiting_to_load,
        loading,
        ready,
        failed
    };
    
    template <size_t max_vertices, size_t max_indices>
    struct flat_textured_renderer_working_space
    {
        flat_textured_vertex vertices[max_vertices];
        size_t num_vertices = 0;

        flat_textured_vertex * vertex_begin() { return vertices; }
        flat_textured_vertex * vertex_end() { return vertices + max_vertices; }
        flat_textured_vertex * vertex_current() { return vertices + num_vertices; }
        const flat_textured_vertex * vertex_begin() const { return vertices; }
        const flat_textured_vertex * vertex_end() const { return vertices + max_vertices; }
        const flat_textured_vertex * vertex_current() const { return vertices + num_vertices; }

        GLuint indices[max_indices];
        size_t num_indices = 0;

        GLuint * index_begin() { return indices; }
        GLuint * index_end() { return indices + max_indices; }
        GLuint * index_current() { return indices + num_indices; }
        const GLuint * index_begin() const { return indices; }
        const GLuint * index_end() const { return indices + max_indices; }
        const GLuint * index_current() const { return indices + num_indices; }

        void clear()
        {
            num_indices = 0;
            num_vertices = 0;
        }
    };
    
    struct flat_textured_renderer_prepared_data
    {
        vertex_array_resource vbo;
        buffer_resource vertex_buffer;
        buffer_resource index_buffer;
        size_t num_indices = 0;
    };
    
    struct flat_textured_renderer
    {
    private:
        shared_renderer_state & internal_shared_renderer_state;
        
        atl::box2f internal_current_screen_bounds;
        GLuint internal_current_texture;
        
        shader_program_resource internal_program;
        
        flat_textured_renderer_status internal_status;
        file_data internal_vertex_shader_file;
        file_data internal_fragment_shader_file;
        
        // Constants
        // Uniform index.
        enum ShaderUniforms
        {
            UNIFORM_SCREEN_DIM,
            NUM_UNIFORMS
        };
        int internal_shader_uniforms[NUM_UNIFORMS];
        
        enum VertexAttributes
        {
            ATTRIBUTE_POSITION,
            ATTRIBUTE_TEXTURE_COORDINATES,
            ATTRIBUTE_COLOR_MULTIPLY,
            ATTRIBUTE_COLOR_LERP,
            NUM_ATTRIBUTES
        };
        
        void internal_bind_buffers_and_vertex_attributes(const buffer_resource & in_vertex_buffer,
                                                         const buffer_resource & in_index_buffer);

    public:
        flat_textured_renderer(shared_renderer_state & in_rendererState);
        ~flat_textured_renderer();
        
        flat_textured_renderer_status status() const { return internal_status; }
        void incremental_load(const application_folder & in_application_folder);
        
        template <size_t max_vertices, size_t max_indices>
        void prepare(flat_textured_renderer_prepared_data & in_data_to_prepare,
                     const flat_textured_renderer_working_space<max_vertices, max_indices> & in_data,
                     const bool in_use_static_buffers)
        {
            prepare(in_data_to_prepare,
                    in_data.vertex_begin(),
                    in_data.index_begin(),
                    in_data.num_vertices,
                    in_data.num_indices,
                    in_use_static_buffers);
        }
        
        void prepare(flat_textured_renderer_prepared_data & in_data_to_prepare,
                     const flat_textured_vertex * in_vertices,
                     const GLuint * in_indices,
                     const size_t in_num_vertices,
                     const size_t in_num_indices,
                     const bool in_use_static_buffers);
        
        void render(const flat_textured_renderer_prepared_data & in_prepared_data,
                    const GLuint in_texture);
    };
}
