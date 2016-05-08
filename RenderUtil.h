

#pragma once

#include "ATF/NomRenderingHeaders.h"
#include <string>

class NomFiles;

namespace RenderUtil
{
    bool compileShader(const NomFiles & in_files, GLuint* shader,  GLenum type, const std::string & in_filePath);
    bool linkProgram(GLuint prog);
    bool validateProgram(GLuint prog);
}
