/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: VBuffer.h/cpp
 Desc: An OpenGL VBO (Vertex Buffer Object) wrapper. Allows run-
 time access to geometry properties of the list. Wraps / contains
 vertex data, color data, and texture data. Note that a VBO can
 only refer to one texture, passed in the constructor. Also note
 that the "GeometryType" can only be of type GL_POINTS,
 GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP,
 GL_TRIANGLE_FAN, GL_TRIANGLES, GL_QUAD_STRIP, GL_QUADS, and
 GL_POLYGON.
 
***************************************************************/

// Inclusion guard
#ifndef __VBUFFER_H__
#define __VBUFFER_H__

#include "MUtil.h"
#include "Queue.h"
#include "Vector2.h"
#include "Vector3.h"

// Number of floats per vertex entry ((x,y,z)(u,v)(r,g,b))
static const int VBuffer_FloatsPerVertex = 8;

class VBuffer
{
public:
    
    // Construct & release buffers
    VBuffer(GLuint GeometryType, GLuint TextureID);
    ~VBuffer();
    
    // Specialty constructor; takes the config-file name to load both a *.obx and texture file
    VBuffer(const char* ObxFile);
    
    // Add a vertex to the object
    void AddVertex(Vector3<float> VertexPos, Vector3<float> ColorVal, Vector2<float> TexturePos);
    
    // Clear all vertices
    void Clear();
    
    // Generate actual VBO (releasing any previous object in memory)
    void Generate();
    
    // Render object
    void Render();
    
    // Returns true if there is no geometry content
    bool IsEmpty();
    
private:
    
    // Working buffer of vertex, color, and texture data (all in parallel)
    Queue< Vector3<float> > VertexPositions;
    Queue< Vector3<float> > ColorValues;
    Queue< Vector2<float> > TexturePositions;
    
    // OpenGL VBO index, geometry type, and texture ID
    GLuint BufferID, GeometryType, TextureID;
    
    // Total number of vertices in the vertex buffer
    int VertexCount;
};

#endif
