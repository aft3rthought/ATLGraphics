

#include "NomFlatColorGeoRenderer.h"

#include "ATLUtil/debug_break.h"
#include "NomRendererState.h"
#include "RenderUtil.h"

namespace atl_graphics_namespace_config
{
    flat_color_geometry_renderer::flat_color_geometry_renderer(shared_renderer_state & in_rendererState)
    :
    internal_shared_renderer_state(in_rendererState),
    internal_status(flat_color_geometry_renderer_status::waiting_to_load)
    {}

    void flat_color_geometry_renderer::incremental_load(const application_folder & in_application_folder)
    {
        switch(internal_status)
        {
            case flat_color_geometry_renderer_status::ready:
            case flat_color_geometry_renderer_status::failed:
                break;
            case flat_color_geometry_renderer_status::waiting_to_load:
            {
                internal_vertex_shader_file.load(in_application_folder, "FlatColorGeo.vsh");
                internal_fragment_shader_file.load(in_application_folder, "FlatColorGeo.fsh");
                internal_status = flat_color_geometry_renderer_status::loading;
                break;
            }
            case flat_color_geometry_renderer_status::loading:
            {
                auto l_vsh_status = internal_vertex_shader_file.status();
                auto l_fsh_status = internal_fragment_shader_file.status();
                if((l_vsh_status != file_data_status::loading &&
                    l_vsh_status != file_data_status::ready) ||
                    (l_fsh_status != file_data_status::loading &&
                     l_fsh_status != file_data_status::ready))
                {
                    internal_status = flat_color_geometry_renderer_status::failed;
                }
                else if(l_vsh_status == file_data_status::ready &&
                        l_fsh_status == file_data_status::ready)
                {
                    GLuint l_vertex_shader = 0, l_fragment_shader = 0;

                    if(!compile_shader(internal_vertex_shader_file.data(), &l_vertex_shader, GL_VERTEX_SHADER) ||
                       !compile_shader(internal_fragment_shader_file.data(), &l_fragment_shader, GL_FRAGMENT_SHADER))
                    {
                        atl_break_debug("Shader failed to compile");
                        internal_status = flat_color_geometry_renderer_status::failed;
                    }
                    else
                    {
                        // Create shader program.
                        internal_program_gl_handle.alloc();

                        // Attach vertex shader to program.
                        glAttachShader(internal_program_gl_handle, l_vertex_shader);
                        check_gl_errors();

                        // Attach fragment shader to program.
                        glAttachShader(internal_program_gl_handle, l_fragment_shader);
                        check_gl_errors();

                        // Bind attribute locations.
                        // This needs to be done prior to linking.
                        glBindAttribLocation(internal_program_gl_handle, ATTRIBUTE_VERT_POSITION, "v_vertPosition");
                        check_gl_errors();
                        glBindAttribLocation(internal_program_gl_handle, ATTRIBUTE_VERT_COLOR, "v_vertColor");
                        check_gl_errors();

                        // Link program.
                        if(link_program(internal_program_gl_handle))
                        {
                            // Get uniform locations.
                            pm_shaderUniforms[UNIFORM_SCREEN_DIM] = glGetUniformLocation(internal_program_gl_handle, "u_screenBounds");
                            pm_shaderUniforms[UNIFORM_COLOR] = glGetUniformLocation(internal_program_gl_handle, "u_color");
                            pm_shaderUniforms[UNIFORM_SCALE] = glGetUniformLocation(internal_program_gl_handle, "u_scale");
                            pm_shaderUniforms[UNIFORM_OFFSET] = glGetUniformLocation(internal_program_gl_handle, "u_offset");

                            internal_status = flat_color_geometry_renderer_status::ready;
                        }
                        else
                        {
                            atl_break_debug("Shader failed to link");
                            internal_program_gl_handle.free();
                            internal_status = flat_color_geometry_renderer_status::failed;
                        }
                    }

                    internal_vertex_shader_file.free();
                    internal_fragment_shader_file.free();

                    // Release vertex and fragment shaders.
                    if(l_vertex_shader != 0)
                    {
                        glDetachShader(internal_program_gl_handle, l_vertex_shader);
                        glDeleteShader(l_vertex_shader);
                    }
                    if(l_fragment_shader != 0)
                    {
                        glDetachShader(internal_program_gl_handle, l_fragment_shader);
                        glDeleteShader(l_fragment_shader);
                    }
                }
                break;
            }
        }
    }

    flat_color_geometry_renderer::~flat_color_geometry_renderer()
    {
        internal_program_gl_handle.free();
    }

    void flat_color_geometry_renderer::internal_configure_vertex_array(const buffer_resource & in_vertexBuffer, const buffer_resource & in_indexBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, in_vertexBuffer);
        check_gl_errors();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, in_indexBuffer);
        check_gl_errors();

        glEnableVertexAttribArray(ATTRIBUTE_VERT_POSITION);
        check_gl_errors();
        glVertexAttribPointer(ATTRIBUTE_VERT_POSITION, 2, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset_start(flat_color_geometry_renderer::Vertex));
        check_gl_errors();

        glEnableVertexAttribArray(ATTRIBUTE_VERT_COLOR);
        check_gl_errors();
        glVertexAttribPointer(ATTRIBUTE_VERT_COLOR, 4, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset(flat_color_geometry_renderer::Vertex, color));
        check_gl_errors();
    }

    void flat_color_geometry_renderer::prepareBuffers(const vertex_array_resource & in_vertexArray,
                                                      const buffer_resource & in_vertexBuffer,
                                                      const buffer_resource & in_indexBuffer,
                                                      const std::vector<Vertex> & in_vertices,
                                                      const std::vector<Tri> & in_triangles,
                                                      const bool in_useStaticBuffers)
    {
        atl_assert_debug(in_vertexBuffer.valid(), "Allocate this ahead of time!");
        atl_assert_debug(in_indexBuffer.valid(), "Allocate this ahead of time!");

        in_vertexArray.bind();

        internal_configure_vertex_array(in_vertexBuffer, in_indexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * in_vertices.size(), in_vertices.data(), in_useStaticBuffers ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
        check_gl_errors();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Tri) * in_triangles.size(), in_triangles.data(), GL_STATIC_DRAW);
        check_gl_errors();

        in_vertexArray.unbind();
    }

    void flat_color_geometry_renderer::render(const vertex_array_resource & in_vbo,
                                              const buffer_resource & in_vertexBuffer,
                                              const buffer_resource & in_indexBuffer,
                                              const int in_numTriangles,
                                              const atl::color & in_color)
    {
        render(in_vbo,
               in_vertexBuffer,
               in_indexBuffer,
               in_numTriangles,
               in_color,
               atl::size2f::Identity,
               atl::point2f(0.f, 0.f));
    }

    void flat_color_geometry_renderer::render(const vertex_array_resource & in_vbo,
                                              const buffer_resource & in_vertexBuffer,
                                              const buffer_resource & in_indexBuffer,
                                              const int in_numTriangles,
                                              const atl::color & in_color,
                                              const atl::size2f & in_scale,
                                              const atl::point2f & in_offset)
    {
        if(internal_shared_renderer_state.setAsCurrentRenderer(this))
        {
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            glUseProgram(internal_program_gl_handle);
            check_gl_errors();

            glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                        internal_shared_renderer_state.m_currentBounds.l,
                        internal_shared_renderer_state.m_currentBounds.b,
                        internal_shared_renderer_state.m_currentBounds.width(),
                        internal_shared_renderer_state.m_currentBounds.height());
            check_gl_errors();
        }

        glUniform4f(pm_shaderUniforms[UNIFORM_COLOR], in_color.r, in_color.g, in_color.b, in_color.a);
        check_gl_errors();
        glUniform2f(pm_shaderUniforms[UNIFORM_SCALE], in_scale.w, in_scale.h);
        check_gl_errors();
        glUniform2f(pm_shaderUniforms[UNIFORM_OFFSET], in_offset.x, in_offset.y);
        check_gl_errors();

        if(in_vbo.bind())
        {
            glBindBuffer(GL_ARRAY_BUFFER, in_vertexBuffer);
            check_gl_errors();
        }
        else
        {
            internal_configure_vertex_array(in_vertexBuffer, in_indexBuffer);
        }

        glDrawElements(GL_TRIANGLES, in_numTriangles * 3, GL_UNSIGNED_INT, (void*)0);
        check_gl_errors();
    }

    void flat_color_geometry_renderer::renderVerts(const vertex_array_resource & in_vbo,
                                                   const buffer_resource & in_vertexBuffer,
                                                   const buffer_resource & in_indexBuffer,
                                                   const int in_numTriangles,
                                                   const std::vector<Vertex> & in_vertices)
    {
        renderVerts(in_vbo, in_vertexBuffer, in_indexBuffer, in_numTriangles, in_vertices, atl::color_white, atl::size2f::Identity, atl::point2f::Zero);
    }

    void flat_color_geometry_renderer::renderVerts(const vertex_array_resource & in_vbo,
                                                   const buffer_resource & in_vertexBuffer,
                                                   const buffer_resource & in_indexBuffer,
                                                   const int in_numTriangles,
                                                   const std::vector<Vertex> & in_vertices,
                                                   const atl::color & in_color,
                                                   const atl::size2f & in_scale,
                                                   const atl::point2f & in_offset)
    {
        if(internal_shared_renderer_state.setAsCurrentRenderer(this))
        {
            glUseProgram(internal_program_gl_handle);
            check_gl_errors();

            glUniform4f(pm_shaderUniforms[UNIFORM_SCREEN_DIM],
                        internal_shared_renderer_state.m_currentBounds.l,
                        internal_shared_renderer_state.m_currentBounds.b,
                        internal_shared_renderer_state.m_currentBounds.width(),
                        internal_shared_renderer_state.m_currentBounds.height());
            check_gl_errors();
        }

        glUniform4f(pm_shaderUniforms[UNIFORM_COLOR], in_color.r, in_color.g, in_color.b, in_color.a);
        check_gl_errors();
        glUniform2f(pm_shaderUniforms[UNIFORM_SCALE], in_scale.w, in_scale.h);
        check_gl_errors();
        glUniform2f(pm_shaderUniforms[UNIFORM_OFFSET], in_offset.x, in_offset.y);
        check_gl_errors();

        if(in_vbo.bind())
        {
            glBindBuffer(GL_ARRAY_BUFFER, in_vertexBuffer);
            check_gl_errors();
        }
        else
        {
            internal_configure_vertex_array(in_vertexBuffer, in_indexBuffer);
        }


        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * in_vertices.size(), in_vertices.data());
        check_gl_errors();

        glDrawElements(GL_TRIANGLES, in_numTriangles * 3, GL_UNSIGNED_INT, (void*)0);
        check_gl_errors();
    }
}
