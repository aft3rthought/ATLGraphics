

#include "ATF/framebuffer.h"
#include "ATF/NomRendererState.h"

namespace atl_graphics_namespace_config
{
    void make_framebuffer(framebuffer_descriptor & descriptor)
    {
        if(!descriptor.valid)
        {
            GLint color_format = GL_R;
            switch(descriptor.color_mode)
            {
                default:
                case atl::framebuffer_color_mode::single_channel: color_format = GL_R; break;
                case atl::framebuffer_color_mode::rgb: color_format = GL_RGB; break;
                case atl::framebuffer_color_mode::rgba: color_format = GL_RGBA; break;
            }

            GLint color_filtering_mode = GL_LINEAR;
            switch(descriptor.filtering_mode)
            {
                default:
                case atl::texture_filtering_mode::linear: color_filtering_mode = GL_LINEAR; break;
                case atl::texture_filtering_mode::nearest: color_filtering_mode = GL_NEAREST; break;
            }
            
            GLuint framebuffer_name = 0;
            glGenFramebuffers(1, &framebuffer_name);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_name);
            
            GLuint texture_name = 0;
            glGenTextures(1, &texture_name);
            glBindTexture(GL_TEXTURE_2D, texture_name);
            glTexImage2D(GL_TEXTURE_2D, 0, color_format, descriptor.width, descriptor.height, 0, color_format, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, color_filtering_mode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, color_filtering_mode);
            glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_name, 0);

            /*
            if(descriptor.depth_buffer_size > 0)
            {
                GLuint depth_buffer_name = 0;
                glGenRenderbuffers(1, &depth_buffer_name);
                glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_name);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, descriptor.width, descriptor.height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_name);
            }
             */
            
            GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, DrawBuffers);
            
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
            {
                descriptor.framebuffer_name = framebuffer_name;
                descriptor.texture_name = texture_name;
                descriptor.valid = true;
            }
        }
    }
    
    framebuffer_scope::framebuffer_scope(const framebuffer_descriptor & descriptor, shared_renderer_state & in_renderer_state)
    :
    framebuffer_scope(descriptor, in_renderer_state, atl::box2f(descriptor.height, descriptor.width, 0.f, 0.f))
    {}
    
    framebuffer_scope::framebuffer_scope(const framebuffer_descriptor & descriptor, shared_renderer_state & in_renderer_state, const atl::box2f & in_stage_bounds)
    :
    renderer_state(in_renderer_state)
    {
        glGetIntegerv(GL_VIEWPORT, original_viewport);
        original_screen_bounds = renderer_state.m_currentBounds;
        if(descriptor.valid)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, descriptor.framebuffer_name);
            glViewport(0, 0, descriptor.width, descriptor.height);
            renderer_state.setAsCurrentRenderer(nullptr);
            renderer_state.m_currentBounds = in_stage_bounds;
        }
    }
    
    framebuffer_scope::~framebuffer_scope()
    {
        renderer_state.m_currentBounds = original_screen_bounds;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(original_viewport[0],
                   original_viewport[1],
                   original_viewport[2],
                   original_viewport[3]);
    }
}
