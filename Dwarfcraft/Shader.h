/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Shader.h/cpp
 Desc: Trivial shader wrapper and accessor. Note that we use the
 classic ARB functions rather than OpenGL 2.0 values for better
 debugging support (the logging function only uses the classic
 ARB Shader reference pointer)
 
***************************************************************/

// Inclusion guard
#ifndef __SHADER_H__
#define __SHADER_H__

#include "MUtil.h"
#include "Queue.h"

class Shader
{
public:
    
    // Takes the vertex and then fragment shader (both are required)
    Shader(const char* VertexName, const char* FragName);
    
    // Releases all internal OpenGL bindings
    ~Shader();
    
    // Activate and deactivate
    void Activate();
    void Deactivate();
    
    // Variable accessors
    void Uniform(const char* Key, GLint value);
    void Uniform(const char* Key, GLfloat value);
    void Uniform(const char* Key, GLfloat v0, GLfloat v1);
    void Uniform(const char* Key, GLfloat v0, GLfloat v1, GLfloat v2);
    void Uniform(const char* Key, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void Uniform(const char* Key, GLsizei count, const GLfloat *array);
    void Uniform3v(const char* Key, const GLfloat *array);
    void UniformMatrix3(const char* Key, const GLfloat *array);
    void Uniform4v(const char* Key, const GLfloat *array);
    void UniformMatrix4(const char* Key, const GLfloat *array);
    
private:
    
    // Check for errors by calling the internal GLSL logging
    void ErrorCheck(GLuint Handle);
    
    // Shader handles
    GLuint VertexGLSL, FragGLSL, ShaderHandle;
};

#endif
