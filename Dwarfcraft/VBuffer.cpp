/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "VBuffer.h"

VBuffer::VBuffer(GLuint GeometryType, GLuint TextureID)
{
    // Save the geometry type used for rendering and the texture ID
    this->GeometryType = GeometryType;
    this->TextureID = TextureID;
    
    // Default to no allocation
    BufferID = 0;
    VertexCount = -1;
}

VBuffer::~VBuffer()
{
    // Release all data
    Clear();
}

VBuffer::VBuffer(const char* ObxFile)
{
    /*** Regular Initialize ***/
    
    // Save the geometry type used for rendering and the texture ID
    GeometryType = GL_TRIANGLES;
    TextureID = -1;
    
    // Default to no allocation
    BufferID = 0;
    VertexCount = -1;
    
    /*** Load Data ***/
    
    // Load the given configuration file and find the model and texture name
    g2Config Config;
    Config.LoadFile(ObxFile);
    
    char* ModelFileName;
    Config.GetValue("General", "Model", &ModelFileName);
    
    char* TextureFileName;
    Config.GetValue("General", "Texture", &TextureFileName);
    
    // Load the model file and push all the vertices
    FILE* ModelData = fopen(ModelFileName, "r");
    UtilAssert(ModelData != NULL, "Unable to load \"%s\" as the source model data", ModelFileName);
    
    // Keep reading until eof
    Vector3<float> Pos;
    Vector2<float> UV;
    while(fscanf(ModelData, "%f %f %f %f %f", &Pos.x, &Pos.y, &Pos.z, &UV.x, &UV.y) == 5)
    {
        // Invert UV because of texture offsets
        UV.y = 1.0f - UV.y;
        
        // Add vertex with white color
        AddVertex(Pos, Vector3<float>(1, 1, 1), UV);
    }
    fclose(ModelData);
    
    // Build model
    Generate();
    
    // Load the texture
    TextureID = g2LoadImage(TextureFileName, NULL, NULL, NULL, false, false);
    if(TextureID == UINT_MAX)
        TextureID = 0;
}

void VBuffer::AddVertex(Vector3<float> VertexPos, Vector3<float> ColorPos, Vector2<float> TexturePos)
{
    VertexPositions.Enqueue(VertexPos);
    ColorValues.Enqueue(ColorPos);
    TexturePositions.Enqueue(TexturePos);
}

void VBuffer::Clear()
{
    // Release all data buffer
    while(!VertexPositions.IsEmpty())
        VertexPositions.Dequeue();
    while(!ColorValues.IsEmpty())
        ColorValues.Dequeue();
    while(!TexturePositions.IsEmpty())
        TexturePositions.Dequeue();
    
    // Release the VBO itself
    glDeleteBuffers(1, &BufferID);
    VertexCount = -1;
}

void VBuffer::Generate()
{
    // Release the VBO itself
    glDeleteBuffers(1, &BufferID);
    
    // Don't generate if no data
    if(VertexPositions.IsEmpty())
    {
        BufferID = -1;
        return;
    }
    
    // Number of vertices
    VertexCount = (int)VertexPositions.GetSize();
    float* Vertices = new float[VertexCount * VBuffer_FloatsPerVertex];
    
    // Copy over each element's sub structures of ((x,y,z)(u,v)(r,g,b))
    int Index = 0;
    for(int i = 0; i < VertexCount; i++)
    {
        Vector3<float> VertexPos = VertexPositions.Dequeue();
        Vector2<float> TexturePos = TexturePositions.Dequeue();
        Vector3<float> ColorPos = ColorValues.Dequeue();
        
        Vertices[Index++] = VertexPos.x; Vertices[Index++] = VertexPos.y; Vertices[Index++] = VertexPos.z;
        Vertices[Index++] = TexturePos.x; Vertices[Index++] = TexturePos.y;
        Vertices[Index++] = ColorPos.x; Vertices[Index++] = ColorPos.y; Vertices[Index++] = ColorPos.z;
    }
    
    // Ask for a vertex buffer and copy into it
    glGenBuffers(1, &BufferID);
    glBindBuffer(GL_ARRAY_BUFFER, BufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VertexCount * VBuffer_FloatsPerVertex, (void*)Vertices, GL_STATIC_DRAW);
    
    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Release internal memory
    delete[] Vertices;
}

void VBuffer::Render()
{
    // Ignore if not yet generated
    if(BufferID == 0 || VertexCount <= 0)
        return;
    
    // Enable texture
    if(TextureID > 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, TextureID);
    }
    
    // Turn on client states
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    // Bind VBO as the active vertex data and vertex index
    glBindBuffer(GL_ARRAY_BUFFER, BufferID);
    
    // Set the vertex structure, which is a repeated pattern
    // of [(x,y,z),(u,v),(r,g,b)] with the first tuple being the actual
    // vertices (each element being a float)
    glTexCoordPointer(2, GL_FLOAT, sizeof(float) * VBuffer_FloatsPerVertex, (char *)NULL + (sizeof(float) * 3));
    
    // Define color
    glColorPointer(3, GL_FLOAT, sizeof(float) * VBuffer_FloatsPerVertex, (char *)NULL + (sizeof(float) * 5));
    
    // Define vertices
    glVertexPointer(3, GL_FLOAT, sizeof(float) * VBuffer_FloatsPerVertex, 0);
    
    // Render based on a face index system
    // Number of faces, not total floats (4 vertices per face)
    glDrawArrays(GeometryType, 0, VertexCount);
    
    // Done dwaring VBOs
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    // Switch back to regular pointer operations
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Turn off texturing
    if(TextureID > 0)
        glDisable(GL_TEXTURE_2D);
}

bool VBuffer::IsEmpty()
{
    return VertexPositions.IsEmpty();
}
