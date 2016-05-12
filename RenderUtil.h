

#pragma once

#include "ATF/NomRenderingHeaders.h"
#include <vector>

namespace atl
{
    bool compile_shader(const std::vector<unsigned char> & data , GLuint* shader, GLenum type);
    bool link_program(GLuint prog);
    bool validate_program(GLuint prog);
}

#define atl_graphics_vertex_offset_start(__VertexType__) sizeof(__VertexType__), 0
#define atl_graphics_vertex_offset(__VertexType__, __Member__) sizeof(__VertexType__), (const GLvoid*)offsetof(__VertexType__, __Member__)