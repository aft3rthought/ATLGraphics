

#include "ATF/framebuffer.h"
#include "ATF/NomRendererState.h"

namespace atl_graphics_namespace_config
{
    void make_framebuffer(framebuffer_descriptor & descriptor)
    {
        if(!descriptor.valid)
        {
            // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
            GLuint FramebufferName = 0;
            glGenFramebuffers(1, &FramebufferName);
            glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
            
            //Now we need to create the texture which will contain the RGB output of our shader. This code is very classic :
            
            // The texture we're going to render to
            GLuint renderedTexture;
            glGenTextures(1, &renderedTexture);
            
            // "Bind" the newly created texture : all future texture functions will modify this texture
            glBindTexture(GL_TEXTURE_2D, renderedTexture);
            
            // Give an empty image to OpenGL ( the last "0" )
            switch(descriptor.color_mode)
            {
                case atl::framebuffer_color_mode::single_channel:
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_R, descriptor.width, descriptor.height, 0, GL_R, GL_UNSIGNED_BYTE, 0);
                    break;
                }
                case atl::framebuffer_color_mode::rgb:
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, descriptor.width, descriptor.height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
                    break;
                }
                case atl::framebuffer_color_mode::rgba:
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, descriptor.width, descriptor.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                    break;
                }
            }
            
            // Poor filtering. Needed !
            switch(descriptor.filtering_mode)
            {
                case atl::texture_filtering_mode::linear:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    break;
                }
                case atl::texture_filtering_mode::nearest:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    break;
                }
            }
            
            //We also need a depth buffer. This is optional, depending on what you actually need to draw in your texture; but since we’re going to render Suzanne, we need depth-testing.
            /*
             // The depth buffer
             GLuint depthrenderbuffer;
             glGenRenderbuffers(1, &depthrenderbuffer);
             glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
             glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
             glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
             */
            
            //Finally, we configure our framebuffer
            // Set "renderedTexture" as our colour attachement #0
            glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
            
            // Set the list of draw buffers.
            GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
            
            //Something may have gone wrong during the process, depending on the capabilities of the GPU. This is how you check it :
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
            {
                descriptor.framebuffer_name = FramebufferName;
                descriptor.texture_name = renderedTexture;
                descriptor.valid = true;
            }
        }
    }
    
    framebuffer_scope::framebuffer_scope(const framebuffer_descriptor & descriptor, shared_renderer_state & in_renderer_state)
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
            renderer_state.m_currentBounds = atl::box2f(descriptor.height, descriptor.width, 0.f, 0.f);
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
