
#include "ATLUtil/debug_break.h"
#include "ATF/RenderUtil.h"
#include <algorithm>

namespace atl
{
    bool compile_shader(const std::vector<unsigned char> & data, GLuint* shader_gl_handle, GLenum type)
    {
        GLint status;

        auto modified_data = data;
        {
            // Zero terminate:
            modified_data.emplace_back(char(0));

            // On OSX we need to prefix the shader with some junk:
#ifdef PLATFORM_OSX
            {
                const char * l_shader_prefix("#version 120\n#define lowp\n#define mediump\n#define highp\n#define texture2DLodEXT texture2DLod\n");
                const size_t l_shader_prefix_len = strlen(l_shader_prefix);
                
                modified_data.reserve(modified_data.size() + l_shader_prefix_len);
                std::move(modified_data.begin(), modified_data.end(), modified_data.begin() + l_shader_prefix_len);
                for(int i = 0; i < l_shader_prefix_len; i++)
                    modified_data[i] = l_shader_prefix[i];
            }
#endif

            // Compile in openGL:
            {
                const GLchar * shaderDataPtr = (const GLchar *)modified_data.data();

                *shader_gl_handle = glCreateShader(type);
                if(*shader_gl_handle == 0)
                    return false;

                glShaderSource(*shader_gl_handle, 1, &shaderDataPtr, NULL);
                glCompileShader(*shader_gl_handle);
            }
        }

        //
#if defined(DEBUG)
        GLint l_logLength;
        glGetShaderiv(*shader_gl_handle, GL_INFO_LOG_LENGTH, &l_logLength);
        if(l_logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(l_logLength);
            glGetShaderInfoLog(*shader_gl_handle, l_logLength, &l_logLength, log);
            printf("------------------\nShader Compile Warnings + Errors\nShader: %d (%s)\n%s\n------------------\n", *shader_gl_handle, type == GL_VERTEX_SHADER ? "Vertex" : "Pixel", log);
            free(log);
        }
#endif

        glGetShaderiv(*shader_gl_handle, GL_COMPILE_STATUS, &status);
        if(status == 0)
        {
            glDeleteShader(*shader_gl_handle);
            return false;
        }
        return true;
    }

    bool link_program(GLuint prog)
    {
        GLint status;
        glLinkProgram(prog);

#if defined(DEBUG)
        GLint logLength;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
        if(logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(prog, logLength, &logLength, log);
            printf("Program link log:\n%s", log);
            free(log);
            atl_fatal("Encountered shader compilation warning/error!");
        }
#endif

        glGetProgramiv(prog, GL_LINK_STATUS, &status);
        if(status == 0) {
            return false;
        }

        return true;
    }

    bool validate_program(GLuint prog)
    {
        GLint logLength, status;

        glValidateProgram(prog);
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
        if(logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(prog, logLength, &logLength, log);
            printf("Program validate log:\n%s", log);
            free(log);
        }

        glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
        if(status == 0) {
            return false;
        }

        return true;
    }
}
