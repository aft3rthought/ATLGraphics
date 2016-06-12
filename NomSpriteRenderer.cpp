

#include "./NomSpriteRenderer.h"

#include "ATLUtil/array_util.h"
#include "ATLUtil/debug_break.h"
#include "ATF/RenderUtil.h"
#include "ATF/NomSpriteFrame.h"
#include "ATF/NomRendererState.h"

namespace atl_graphics_namespace_config
{
    sprite_renderer::sprite_renderer(shared_renderer_state & in_rendererState)
    :
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
                                std::numeric_limits<float>::max()),
    internal_status(sprite_renderer_status::waiting_to_load)
    {}

    void sprite_renderer::incremental_load(const application_folder & in_application_folder)
    {
        switch(internal_status)
        {
            case sprite_renderer_status::ready:
            case sprite_renderer_status::failed:
                break;
            case sprite_renderer_status::waiting_to_load:
            {
                internal_vertex_shader_file.load(in_application_folder, "Sprite.vsh");
                internal_fragment_shader_file.load(in_application_folder, "Sprite.fsh");
                internal_status = sprite_renderer_status::loading;
                break;
            }
            case sprite_renderer_status::loading:
            {
                auto l_vsh_status = internal_vertex_shader_file.status();
                auto l_fsh_status = internal_fragment_shader_file.status();
                if((l_vsh_status != file_data_status::loading &&
                   l_vsh_status != file_data_status::ready) ||
                   (l_fsh_status != file_data_status::loading &&
                   l_fsh_status != file_data_status::ready))
                {
                    internal_status = sprite_renderer_status::failed;
                }
                else if(l_vsh_status == file_data_status::ready &&
                        l_fsh_status == file_data_status::ready)
                {
                    GLuint l_vertex_shader = 0, l_fragment_shader = 0;

                    if(!compile_shader(internal_vertex_shader_file.data(), &l_vertex_shader, GL_VERTEX_SHADER) ||
                       !compile_shader(internal_fragment_shader_file.data(), &l_fragment_shader, GL_FRAGMENT_SHADER))
                    {
                        atl_fatal("Shader failed to compile");
                        internal_status = sprite_renderer_status::failed;
                    }
                    else
                    {
                        SpriteVert cl_spriteVerts[4] =
                        {
                            {1.f, 0.f, 0.f, 0.f},   // c1, l, t    // 0
                            {0.f, 0.f, 0.f, 1.f},   // c4, l, b    // 3
                            {0.f, 1.f, 0.f, 0.f},   // c2, r, t    // 1
                            {0.f, 0.f, 1.f, 0.f},   // c3, r, b    // 2
                        };

                        pm_vertexArray.alloc();
                        pm_vertexBuffer.alloc();

                        pm_vertexArray.bind();
                        internal_configure_vertex_array();
                        pm_vertexArray.unbind();

                        glBufferData(GL_ARRAY_BUFFER, atl::c_array_byte_length(cl_spriteVerts), cl_spriteVerts, GL_STATIC_DRAW);

                        // Create shader program.
                        pm_program.alloc();

                        // Attach vertex shader to program.
                        glAttachShader(pm_program, l_vertex_shader);

                        // Attach fragment shader to program.
                        glAttachShader(pm_program, l_fragment_shader);

                        // Bind attribute locations.
                        // This needs to be done prior to linking.
                        glBindAttribLocation(pm_program, ATTRIBUTE_VERT_INDEXES, "vertComponents");

                        // Link program.
                        if(link_program(pm_program))
                        {
                            // Get uniform locations.
                            pm_shaderUniforms[UNIFORM_SPRITE_CORNERS] = glGetUniformLocation(pm_program, "spriteCorners");
                            pm_shaderUniforms[UNIFORM_SPRITE_TEXCOORDS] = glGetUniformLocation(pm_program, "spriteTexCoords");
                            pm_shaderUniforms[UNIFORM_SCREEN_DIM] = glGetUniformLocation(pm_program, "u_screenBounds");
                            pm_shaderUniforms[UNIFORM_COLOR_MODULATION] = glGetUniformLocation(pm_program, "colorModulation");
                            pm_shaderUniforms[UNIFORM_COLOR_LERP] = glGetUniformLocation(pm_program, "colorLerp");

                            internal_status = sprite_renderer_status::ready;
                        }
                        else
                        {
                            atl_fatal("Shader failed to link");
                            pm_program.free();
                            internal_status = sprite_renderer_status::failed;
                        }
                    }

                    internal_vertex_shader_file.free();
                    internal_fragment_shader_file.free();

                    // Release vertex and fragment shaders.
                    if(l_vertex_shader != 0)
                    {
                        glDetachShader(pm_program, l_vertex_shader);
                        glDeleteShader(l_vertex_shader);
                    }
                    if(l_fragment_shader != 0)
                    {
                        glDetachShader(pm_program, l_fragment_shader);
                        glDeleteShader(l_fragment_shader);
                    }
                }
                break;
            }
        }
    }

    sprite_renderer::~sprite_renderer()
    {
        pm_vertexBuffer.free();
        pm_vertexArray.free();
        pm_program.free();
    }

    void sprite_renderer::internal_configure_vertex_array() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, pm_vertexBuffer);
        check_gl_errors();
        glEnableVertexAttribArray(ATTRIBUTE_VERT_INDEXES);
        check_gl_errors();
        glVertexAttribPointer(ATTRIBUTE_VERT_INDEXES, 4, GL_FLOAT, GL_FALSE, atl_graphics_vertex_offset_start(SpriteVert));
        check_gl_errors();
    }

    quad_position sprite_renderer::transform(const sprite_frame & in_spriteFrame,
                                             const atl::point2f & in_spritePosition,
                                             const atl::size2f & in_spriteScale,
                                             float in_spriteRotation)
    {
        // Apply scale:
        quad_position l_corners(in_spriteFrame.area.l * in_spriteScale.w, in_spriteFrame.area.t * in_spriteScale.h,
                                     in_spriteFrame.area.r * in_spriteScale.w, in_spriteFrame.area.t * in_spriteScale.h,
                                     in_spriteFrame.area.r * in_spriteScale.w, in_spriteFrame.area.b * in_spriteScale.h,
                                     in_spriteFrame.area.l * in_spriteScale.w, in_spriteFrame.area.b * in_spriteScale.h);


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

    void sprite_renderer::renderSprite(const sprite_frame & in_spriteFrame,
                                       const atl::point2f & in_spritePosition,
                                       const atl::size2f & in_spriteScale,
                                       float in_spriteRotation)
    {
        renderSprite(in_spriteFrame, transform(in_spriteFrame, in_spritePosition, in_spriteScale, in_spriteRotation));
    }

    void sprite_renderer::renderSprite(const sprite_frame & in_spriteFrame,
                                       const atl::point2f & in_spritePosition)
    {
        atl::box2f l_area = in_spriteFrame.area;
        l_area += in_spritePosition;

        renderSprite(in_spriteFrame, l_area);
    }

    void sprite_renderer::renderSprite(const sprite_frame & in_spriteFrame,
                                       const atl::point2f & in_spritePosition,
                                       const atl::size2f & in_spriteScale)
    {
        atl::box2f l_area = in_spriteFrame.area;
        l_area += in_spritePosition;
        l_area.x.scale(in_spriteScale.w);
        l_area.y.scale(in_spriteScale.h);

        renderSprite(in_spriteFrame, l_area);
    }

    void sprite_renderer::renderSprite(const sprite_frame & in_spriteFrame,
                                       const quad_position & in_corners)
    {
        renderSprite(in_spriteFrame, in_corners, in_spriteFrame.texture_coordinates);
    }

    void sprite_renderer::renderSprite(const sprite_frame & in_spriteFrame,
                                       const quad_position & in_corners,
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
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            if(pm_vertexArray.bind())
            {
                glBindBuffer(GL_ARRAY_BUFFER, pm_vertexBuffer);
            }
            else
            {
                internal_configure_vertex_array();
            }

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

        if(pm_currentTexture != in_spriteFrame.texture_gl_handle)
        {
            glBindTexture(GL_TEXTURE_2D, in_spriteFrame.texture_gl_handle);
            pm_currentTexture = in_spriteFrame.texture_gl_handle;
        }

        if(pm_currentSpriteCorners != in_corners)
        {
            float l_data[] = {in_corners.c1.x, in_corners.c1.y,
                in_corners.c2.x, in_corners.c2.y,
                in_corners.c3.x, in_corners.c3.y,
                in_corners.c4.x, in_corners.c4.y};

            glUniform2fv(pm_shaderUniforms[UNIFORM_SPRITE_CORNERS], 4, l_data);

            pm_currentSpriteCorners = in_corners;
        }

        if(pm_currentSpriteTexBounds != in_texCoords)
        {
            float l_data[] = {in_texCoords.l, in_texCoords.t,
                in_texCoords.r, in_texCoords.t,
                in_texCoords.r, in_texCoords.b,
                in_texCoords.l, in_texCoords.b};

            glUniform2fv(pm_shaderUniforms[UNIFORM_SPRITE_TEXCOORDS], 4, l_data);

            pm_currentSpriteTexBounds = in_texCoords;
        }

        if(pm_currentColorModulation != pm_colorModulation)
        {
            glUniform4f(pm_shaderUniforms[UNIFORM_COLOR_MODULATION], pm_colorModulation.r, pm_colorModulation.g, pm_colorModulation.b, pm_colorModulation.a);
            pm_currentColorModulation = pm_colorModulation;
        }

        if(pm_currentColorLerp != pm_colorLerp)
        {
            glUniform4f(pm_shaderUniforms[UNIFORM_COLOR_LERP], pm_colorLerp.r, pm_colorLerp.g, pm_colorLerp.b, pm_colorLerp.a);
            pm_currentColorLerp = pm_colorLerp;
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void sprite_renderer::setColorModulation(const atl::color_premul & in_color)
    {
        pm_colorModulation = in_color;
    }

    void sprite_renderer::setColorLerp(const atl::color & in_color)
    {
        pm_colorLerp = in_color;
    }

    void sprite_renderer::clearColorModulation()
    {
        pm_colorModulation.r =
            pm_colorModulation.g =
            pm_colorModulation.b =
            pm_colorModulation.a = 1.f;
    }

    void sprite_renderer::clearColorLerp()
    {
        pm_colorLerp.r =
            pm_colorLerp.g =
            pm_colorLerp.b =
            pm_colorLerp.a = 0.f;
    }
}
