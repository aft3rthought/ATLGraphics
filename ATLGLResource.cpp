

#include "ATLGLResource.h"
#include "ATLUtil/debug_break.h"

ATLGLBaseResource::~ATLGLBaseResource()
{
    static_assert(sizeof(ATLGLBaseResource) == sizeof(GLuint),"Size of ATLGLResource has to be the same as GLuint");
    static_assert(sizeof(ATLGLTexture) == sizeof(GLuint),"Size of ATLGLResource has to be the same as GLuint");
    static_assert(sizeof(ATLGLProgram) == sizeof(GLuint),"Size of ATLGLResource has to be the same as GLuint");
    static_assert(sizeof(ATLGLBuffer) == sizeof(GLuint),"Size of ATLGLResource has to be the same as GLuint");
    static_assert(sizeof(ATLGLVertexArray) == sizeof(GLuint),"Size of ATLGLResource has to be the same as GLuint");
    
    SGDebugBreakIf(isValid(), "GL resource not set to invalid before owner is destroyed! Is a resource being lost?");
}

#ifdef DEBUG
ATLGLBaseResource::operator GLuint () const
{
    SGDebugBreakIf(isInvalid(), "Accesing invalid GL resource");
    return pm_resource;
}
#endif

void ATLGLTexture::alloc()
{
    glGenTextures(1, &pm_resource);
    atf::check_gl_errors();
}

void ATLGLTexture::free()
{
#ifdef DEBUG
    SGDebugBreakIf(!glIsTexture(pm_resource), "Resource isn't valid when calling free, is everything OK?");
#endif
    
    glDeleteTextures(1, &pm_resource);
    atf::check_gl_errors();
    pm_resource = 0;
}

void ATLGLProgram::alloc()
{
    pm_resource = glCreateProgram();
    atf::check_gl_errors();
}

void ATLGLProgram::free()
{
#ifdef DEBUG
    SGDebugBreakIf(!glIsProgram(pm_resource), "Resource isn't valid when calling free, is everything OK?");
#endif
    
    glDeleteProgram(pm_resource);
    atf::check_gl_errors();
    pm_resource = 0;
}

void ATLGLBuffer::alloc()
{
    glGenBuffers(1, &pm_resource);
    atf::check_gl_errors();
}

void ATLGLBuffer::free()
{
#ifdef DEBUG
    SGDebugBreakIf(!glIsBuffer(pm_resource), "Resource isn't valid when calling free, is everything OK?");
#endif
    
    glDeleteBuffers(1, &pm_resource);
    atf::check_gl_errors();
    pm_resource = 0;
}

void ATLGLVertexArray::alloc()
{
    glGenVertexArrays_NOM(1, &pm_resource);
    atf::check_gl_errors();
}

void ATLGLVertexArray::free()
{
#ifdef DEBUG
    SGDebugBreakIf(!glIsVertexArray_NOM(pm_resource), "Resource isn't valid when calling free, is everything OK?");
#endif
    
    glDeleteVertexArrays_NOM(1, &pm_resource);
    atf::check_gl_errors();
    pm_resource = 0;
}