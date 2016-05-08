
#include "ATLUtil/debug_break.h"
#include "ATF/NomFiles.h"
#include "ATF/RenderUtil.h"
#include <fstream>
#include <vector>
#include <algorithm>

bool RenderUtil::compileShader(const NomFiles & in_files, GLuint* in_shader,  GLenum in_type, const std::string & in_shaderName)
{
    GLint status;
    
    {
        std::vector<char> l_shaderData;
        
        // Read from file:
        {
            std::string l_shaderPath = in_files.rootDirectory() + "/" + in_shaderName + (in_type == GL_VERTEX_SHADER ?  ".vsh" : ".fsh");
            
            std::ifstream l_file(l_shaderPath, std::ios::in|std::ios::ate);
            
            if(l_file.good())
            {
                //
                std::streamoff l_size = 0;
                if(l_file.seekg(0, std::ios::end).good())
                    l_size = l_file.tellg();
                
                if(l_file.seekg(0, std::ios::beg).good())
                    l_size -= l_file.tellg();
                
                //
                if(l_size > 0)
                {
                    size_t dataSize = (size_t)l_size;
                    l_shaderData.resize(dataSize);
                    l_file.read(l_shaderData.data(), dataSize);
                }
            }
            
            // Early out if we couldn't load anything:
            if (l_shaderData.empty())
            {
                printf("Failed to load shader: %s\n", l_shaderPath.c_str());
                return false;
            }
        }
        
        // Zero terminate:
        l_shaderData.emplace_back(char(0));
        
        // On OSX we need to prefix the shader with some junk:
#ifdef PLATFORM_OSX
        {
            const std::string cl_shaderPrefix("#version 120\n#define lowp\n#define mediump\n#define highp\n#define texture2DLodEXT texture2DLod\n");

            std::string l_originalShader(l_shaderData.data(), l_shaderData.size());
            std::string l_prefixedShader = cl_shaderPrefix + l_originalShader;
            
            l_shaderData.resize(l_prefixedShader.length());
            std::copy(l_prefixedShader.begin(), l_prefixedShader.end(), l_shaderData.begin());
        }
#endif
        
        // Compile in openGL:
        {
            const GLchar * l_shaderDataPtr = l_shaderData.data();
        
            *in_shader = glCreateShader(in_type);
            glShaderSource(*in_shader, 1, &l_shaderDataPtr, NULL);
            glCompileShader(*in_shader);
        }
    }
    
    //
#if defined(DEBUG)
    GLint l_logLength;
    glGetShaderiv(*in_shader, GL_INFO_LOG_LENGTH, &l_logLength);
    if (l_logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(l_logLength);
        glGetShaderInfoLog(*in_shader, l_logLength, &l_logLength, log);
        printf("------------------\nShader Compile Warnings + Errors\nShader: %s (%s)\n%s\n------------------\n", in_shaderName.c_str(), in_type == GL_VERTEX_SHADER ? "Vertex" : "Pixel", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*in_shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        glDeleteShader(*in_shader);
        return false;
    }
    
    return true;
}

bool RenderUtil::linkProgram(GLuint prog)
{
    GLint status;
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        printf("Program link log:\n%s", log);
        free(log);
        SGDebugBreak("Encountered shader compilation warning/error!");
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0) {
        return false;
    }
    
    return true;
}

bool RenderUtil::validateProgram(GLuint prog)
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        printf("Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0) {
        return false;
    }
    
    return true;
}
