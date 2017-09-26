
#include "flat_textured_renderer.h"

#include "ATF/RenderUtil.h"
#include "ATF/NomRendererState.h"

namespace atl_graphics_namespace_config
{
    flat_textured_renderer::flat_textured_renderer(shared_renderer_state & in_rendererState)
    :
    internal_shared_renderer_state(in_rendererState),
    internal_status(flat_textured_renderer_status::waiting_to_load)
    {}
    
    void flat_textured_renderer::incremental_load(const application_folder & in_application_folder)
    {
        switch(internal_status)
        {
            case flat_textured_renderer_status::ready:
            case flat_textured_renderer_status::failed:
                break;
            case flat_textured_renderer_status::waiting_to_load:
            {
                internal_vertex_shader_file.load(in_application_folder, "flat_textured.vsh");
                internal_fragment_shader_file.load(in_application_folder, "flat_textured.fsh");
                internal_status = flat_textured_renderer_status::loading;
                break;
            }
            case flat_textured_renderer_status::loading:
            {
                auto l_vsh_status = internal_vertex_shader_file.status();
                auto l_fsh_status = internal_fragment_shader_file.status();
                if((l_vsh_status != file_data_status::loading &&
                    l_vsh_status != file_data_status::ready) ||
                   (l_fsh_status != file_data_status::loading &&
                    l_fsh_status != file_data_status::ready))
                {
                    internal_status = flat_textured_renderer_status::failed;
                }
                else if(l_vsh_status == file_data_status::ready &&
                        l_fsh_status == file_data_status::ready)
                {
                    GLuint l_vertex_shader = 0, l_fragment_shader = 0;
                    
                    if(!compile_shader(internal_vertex_shader_file.data(), &l_vertex_shader, GL_VERTEX_SHADER) ||
                       !compile_shader(internal_fragment_shader_file.data(), &l_fragment_shader, GL_FRAGMENT_SHADER))
                    {
                        atl_debug_log("Shader failed to compile");
                        internal_status = flat_textured_renderer_status::failed;
                    }
                    else
                    {
                        // Create shader program.
                        internal_program.alloc();
                        
                        // Attach vertex shader to program.
                        glAttachShader(internal_program, l_vertex_shader);
                        
                        // Attach fragment shader to program.
                        glAttachShader(internal_program, l_fragment_shader);
                        
                        // Bind attribute locations.
                        // This needs to be done prior to linking.
                        glBindAttribLocation(internal_program, ATTRIBUTE_POSITION, "v_position");
                        check_gl_errors();
                        glBindAttribLocation(internal_program, ATTRIBUTE_TEXTURE_COORDINATES, "v_texture_coordinates");
                        check_gl_errors();
                        glBindAttribLocation(internal_program, ATTRIBUTE_COLOR_MULTIPLY, "v_color_multiply");
                        check_gl_errors();
                        glBindAttribLocation(internal_program, ATTRIBUTE_COLOR_LERP, "v_color_lerp");
                        check_gl_errors();

                        // Link program.
                        if(link_program(internal_program))
                        {
                            // Get uniform locations.
                            internal_shader_uniforms[UNIFORM_SCREEN_DIM] = glGetUniformLocation(internal_program, "u_stage_bounds");
                            check_gl_errors();
                            
                            internal_status = flat_textured_renderer_status::ready;
                        }
                        else
                        {
                            atl_debug_log("Shader failed to link");
                            internal_program.free();
                            internal_status = flat_textured_renderer_status::failed;
                        }
                    }
                    
                    internal_vertex_shader_file.free();
                    internal_fragment_shader_file.free();
                    
                    // Release vertex and fragment shaders.
                    if(l_vertex_shader != 0)
                    {
                        glDetachShader(internal_program, l_vertex_shader);
                        glDeleteShader(l_vertex_shader);
                    }
                    if(l_fragment_shader != 0)
                    {
                        glDetachShader(internal_program, l_fragment_shader);
                        glDeleteShader(l_fragment_shader);
                    }
                }
                break;
            }
        }
    }
    
    flat_textured_renderer::~flat_textured_renderer()
    {
        internal_program.free();
    }
    
    void flat_textured_renderer::internal_bind_buffers_and_vertex_attributes(const buffer_resource & in_vertex_buffer,
                                                                             const buffer_resource & in_index_buffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, in_vertex_buffer);
        check_gl_errors();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, in_index_buffer);
        check_gl_errors();

        glEnableVertexAttribArray(ATTRIBUTE_POSITION);
        check_gl_errors();
        glVertexAttribPointer(ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset_start(flat_textured_vertex));
        check_gl_errors();
        glEnableVertexAttribArray(ATTRIBUTE_TEXTURE_COORDINATES);
        check_gl_errors();
        glVertexAttribPointer(ATTRIBUTE_TEXTURE_COORDINATES, 2, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset(flat_textured_vertex, texture_coordinates));
        check_gl_errors();
        glEnableVertexAttribArray(ATTRIBUTE_COLOR_MULTIPLY);
        check_gl_errors();
        glVertexAttribPointer(ATTRIBUTE_COLOR_MULTIPLY, 4, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset(flat_textured_vertex, color_multiply));
        check_gl_errors();
        glEnableVertexAttribArray(ATTRIBUTE_COLOR_LERP);
        check_gl_errors();
        glVertexAttribPointer(ATTRIBUTE_COLOR_LERP, 4, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset(flat_textured_vertex, color_lerp));
        check_gl_errors();
    }
    
    void flat_textured_renderer::prepare(flat_textured_renderer_prepared_data & in_data_to_prepare,
                                         const flat_textured_vertex * in_vertices,
                                         const GLuint * in_indices,
                                         const size_t in_num_vertices,
                                         const size_t in_num_indices,
                                         const bool in_use_static_buffers)
    {
        auto & vbo = in_data_to_prepare.vbo;
        auto & vertex_buffer = in_data_to_prepare.vertex_buffer;
        auto & index_buffer = in_data_to_prepare.index_buffer;
        
        in_data_to_prepare.num_indices = in_num_indices;
        
        if(!vbo.valid()) vbo.alloc();
        if(!vertex_buffer.valid()) vertex_buffer.alloc();
        if(!index_buffer.valid()) index_buffer.alloc();

        if(vertex_buffer.valid() && index_buffer.valid())
        {
            if(vbo.valid()) vbo.bind();
                
            internal_bind_buffers_and_vertex_attributes(vertex_buffer, index_buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(flat_textured_vertex) * in_num_vertices, in_vertices, in_use_static_buffers ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            check_gl_errors();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * in_num_indices, in_indices, GL_STATIC_DRAW);
            check_gl_errors();

            if(vbo.valid()) vbo.unbind();
        }
    }
    
    void flat_textured_renderer::render(const flat_textured_renderer_prepared_data & in_prepared_data,
                                        const GLuint in_texture)
    {
        if(internal_shared_renderer_state.setAsCurrentRenderer(this))
        {
            internal_current_screen_bounds.t =
            internal_current_screen_bounds.r =
            internal_current_screen_bounds.b =
            internal_current_screen_bounds.l = std::numeric_limits<float>::max();
            internal_current_texture = 0;
            
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            
            glUseProgram(internal_program);
            check_gl_errors();
        }
        
        if(internal_current_screen_bounds != internal_shared_renderer_state.m_currentBounds)
        {
            glUniform4f(internal_shader_uniforms[UNIFORM_SCREEN_DIM],
                        internal_shared_renderer_state.m_currentBounds.l,
                        internal_shared_renderer_state.m_currentBounds.b,
                        internal_shared_renderer_state.m_currentBounds.width(),
                        internal_shared_renderer_state.m_currentBounds.height());
            
            internal_current_screen_bounds = internal_shared_renderer_state.m_currentBounds;
        }
        
        if(internal_current_texture != in_texture)
        {
            glBindTexture(GL_TEXTURE_2D, in_texture);
            internal_current_texture = in_texture;
        }
        
        if(in_prepared_data.vbo.bind())
        {
            glBindBuffer(GL_ARRAY_BUFFER, in_prepared_data.vertex_buffer);
            check_gl_errors();
        }
        else
        {
            internal_bind_buffers_and_vertex_attributes(in_prepared_data.vertex_buffer, in_prepared_data.index_buffer);
        }
        
        glDrawElements(GL_TRIANGLES, in_prepared_data.num_indices, GL_UNSIGNED_INT, (void*)0);
        check_gl_errors();
    }
}
