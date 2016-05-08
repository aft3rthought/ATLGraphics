

#pragma once

#include "ATF/NomRenderingHeaders.h"

class ATLGLBaseResource
{
protected:
    GLuint pm_resource;
    
public:
    ATLGLBaseResource() : pm_resource(0) {};
    ~ATLGLBaseResource();
    
#ifdef DEBUG
    operator GLuint () const;
#else
    operator GLuint () const { return pm_resource; }
#endif
    
    bool isValid() const { return pm_resource != 0; }
    bool isInvalid() const { return pm_resource == 0; }
};

class ATLGLTexture : public ATLGLBaseResource
{
public:
    void alloc();
    void free();
};

class ATLGLProgram : public ATLGLBaseResource
{
public:
    void alloc();
    void free();
};

class ATLGLBuffer : public ATLGLBaseResource
{
public:
    void alloc();
    void free();
};

class ATLGLVertexArray : public ATLGLBaseResource
{
public:
    void alloc();
    void free();
};