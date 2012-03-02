/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "Shader.h"

Shader::Shader(const char* VertexName, const char* FragName)
{
    /*** Load Source Files ***/
    
    // Read source code of vertex name
    FILE* VertexSourceFile = fopen(VertexName, "r");
    UtilAssert(VertexSourceFile != NULL, "Unable to load shader file \"%s\"", VertexName);
    
    fseek(VertexSourceFile, 0, SEEK_END);
    int VertexSourceLength = (int)ftell(VertexSourceFile);
    fseek(VertexSourceFile, 0, SEEK_SET);
    
    // Alloc and read
    char* VertexSource = new char[VertexSourceLength + 1];
    fread(VertexSource, 1, VertexSourceLength, VertexSourceFile);
    VertexSource[VertexSourceLength] = 0;
    fclose(VertexSourceFile);
    
    // Same for fragment shader
    FILE* FragSourceFile = fopen(FragName, "r");
    UtilAssert(FragSourceFile != NULL, "Unable to load shader file \"%s\"", FragName);
    
    fseek(FragSourceFile, 0, SEEK_END);
    int FragSourceLength = (int)ftell(FragSourceFile);
    fseek(FragSourceFile, 0, SEEK_SET);
    
    // Alloc and read
    char* FragSource = new char[FragSourceLength + 1];
    fread(FragSource, 1, FragSourceLength, FragSourceFile);
    FragSource[FragSourceLength] = 0;
    fclose(FragSourceFile);
    
    /*** Create, Compile, and Link Shaders ***/
    
    // Vertex shader
    VertexGLSL = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexGLSL, 1, (const char**)&VertexSource, NULL);
    glCompileShader(VertexGLSL);
    
    ErrorCheck(VertexGLSL);
    
    // Frag shader
    FragGLSL = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragGLSL, 1, (const char**)&FragSource, NULL);
    glCompileShader(FragGLSL);
    
    ErrorCheck(VertexGLSL);
    
    /*** Create, Compile, and Link Program ***/
    
    ShaderHandle = glCreateProgram();
    glAttachShader(ShaderHandle, VertexGLSL);
    glAttachShader(ShaderHandle, FragGLSL);
    glLinkProgram(ShaderHandle);
    
    ErrorCheck(ShaderHandle);
}

Shader::~Shader()
{
    glDeleteProgram(ShaderHandle);
}

void Shader::Activate()
{
    // Set this shader's program
    glUseProgram(ShaderHandle);
}

void Shader::Deactivate()
{
    // Turn off shader program
    glUseProgram(0);
}

void Shader::Uniform(const char* Key, GLint value)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniform1i(loc, value);
    
    glUseProgram(oldprog);
}

void Shader::Uniform(const char* Key, GLfloat value)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniform1f(loc, value);
    
    glUseProgram(oldprog);
}

void Shader::Uniform(const char* Key, GLfloat v0, GLfloat v1)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniform2f(loc, v0, v1);
    
    glUseProgram(oldprog);
}

void Shader::Uniform(const char* Key, GLfloat v0, GLfloat v1, GLfloat v2)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniform3f(loc, v0, v1, v2);
    
    glUseProgram(oldprog);
}

void Shader::Uniform(const char* Key, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniform4f(loc, v0, v1, v2, v3);
    
    glUseProgram(oldprog);
}

void Shader::Uniform(const char* Key, GLsizei count, const GLfloat *array)
{ 
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniform1fv(loc, count, array);
    
    glUseProgram(oldprog);
}

void Shader::Uniform3v(const char* Key, const GLfloat *array)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniform3fv(loc, 1, array);
    
    glUseProgram(oldprog);
}

void Shader::UniformMatrix3(const char* Key, const GLfloat *array)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniformMatrix3fv(loc, 1, 0, array);
    
    glUseProgram(oldprog);
}

void Shader::Uniform4v(const char* Key, const GLfloat *array)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniform4fv(loc, 1, array);
    
    glUseProgram(oldprog);
}

void Shader::UniformMatrix4(const char* Key, const GLfloat *array)
{
    GLint loc, oldprog;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldprog);
    glUseProgram(ShaderHandle);
    
    loc = glGetUniformLocation(ShaderHandle, Key);
    glUniformMatrix4fv(loc, 1, 0, array);
    
    glUseProgram(oldprog);
}

void Shader::ErrorCheck(GLuint Handle)
{
    //Attempt to get the length of our error log.
    int LogLength;
    glGetObjectParameterivARB((GLhandleARB)Handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &LogLength);
    if(LogLength <= 0)
        return;
    
    //Create a buffer to read compilation error message
    char* Log = new char[LogLength + 1];
    glGetInfoLogARB((GLhandleARB)Handle, LogLength + 1, NULL, Log);
    if(strlen(Log) <= 0)
        return;
    
    // Print out the details
    UtilAssert(false, "A shader failed to load: \"%s\"", Log);
}
