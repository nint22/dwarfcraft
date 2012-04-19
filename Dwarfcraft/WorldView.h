/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: WorldView.h/cpp
 Desc: Wraps all world volume and generation, dealing mostly with
 user inputs and interaction with the world.
 
***************************************************************/

// Inclusion guard
#ifndef __WORLDVIEW_H__
#define __WORLDVIEW_H__

#include "Globals.h"

#include "MGrfx.h"
#include "WorldContainer.h"
#include "VBuffer.h"
#include "Stack.h"

#include "VolumeView.h"
#include "ItemsView.h"
#include "Entities.h"
#include "StructsView.h"

// A column's layer VBO representation
struct WorldView_Plane
{
    // Cube data
    // No matter what depth, always render
    VBuffer* WorldGeometry;
    
    // Surfaces that are occluded by above layers
    // Only render if intersected layer
    VBuffer* HiddenGeometry;
    
    // The side geometry; hiding world boundaries
    // Only render if intersected layer
    VBuffer* SideGeometry;
};

// A column: a list of planes (0 being bottom, index growing up)
struct WorldView_Column
{
    // A list of layers
    WorldView_Plane* Planes;
};

class WorldView
{
public:
    
    // Constructor and destructor
    WorldView(WorldContainer* WorldData, VolumeView* Designations, ItemsView* Items, StructsView* Structs, Entities* EntitiesList);
    ~WorldView();
    
    // Render the world (no projection changes)
    // Only renders the foward facing chunks; positions are global coordinates, not chunk coordinates
    // The "CameraRight.." variable is the right-facing vector on the x-z plane of the camera
    // The y components are always ignored
    // The camera angle is commonly used when making the 2D sprites face the camera
    void Render(Vector3<float> CameraPos, Vector3<float> CameraRight, int LayerCutoff, float CameraAngle);
    
    // Update the world (mostly used for textures, world effects, etc.)
    void Update(float dT);
    
protected:
    
    // Generate the VBO associated with a column / chunk
    void GenerateColumnVBO(int ChunkX, int ChunkZ);
    
    // Generate a VBO at the given layer
    bool GenerateLayerVBO(int ChunkX, int Y, int ChunkZ, WorldView_Plane* Layer);
    
    // Add a vertex
    void AddVertex(VBuffer* Buffer, Vector3<float> Vertex, Vector3<float> Normal, int QuadCornerIndex, dBlock Block);
    
    // Remove / release all VBOs
    void ClearVBO();
    
private:
    
    /*** World Data ***/
    
    // World data container
    WorldContainer* WorldData;
    
    // Number of chunks in the X dimension (same as Z dimension)
    int ChunkCount;
    
    // How far we render objects up to
    float MaxRenderDist;
    
    /*** Graphical Data ***/
    
    // An array of columns, each column being a renderable structure
    WorldView_Column* Chunks;
    
    /*** Secondary Rendering Elements ***/
    
    // Note: The below references are stringly for rendering only
    
    // Designations list
    VolumeView* Designations;
    
    // Items list
    ItemsView* Items;
    
    // Structs list
    StructsView* Structs;
    
    // Entity list
    Entities* EntitiesList;
};

// End of inclusion guard
#endif
