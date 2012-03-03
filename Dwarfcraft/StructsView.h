/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: StructsView.h/cpp
 Desc: A special world-structures container and renderer. Renders
 / handles all special world objects (such as work benches) which
 are defined as block types in dBlocks.h This class gets called /
 is managed by the world view
 
***************************************************************/

#ifndef __STRUCTSVIEW_H__
#define __STRUCTSVIEW_H__

#include "GrfxObject.h"
#include "List.h"
#include "dBlocks.h"
#include "Vector3.h"
#include "VBuffer.h"
#include "WorldContainer.h"

// Renderable item structure
struct StructsView_Struct
{
    // Base item / structure
    dBlock Struct;
    
    // Current location
    Vector3<int> Pos;
    
    // Total time animation
    float dT;
    
    // Model data
    VBuffer* ModelData;
};

class StructsView
{
public:
    
    // Standard constructor and desctructor
    StructsView(WorldContainer* WorldData);
    ~StructsView();
    
    // Add a struct to the world at the given position
    void AddStruct(dBlock Struct, Vector3<int> Pos);
    
    // Remove a struct from the world
    void RemoveStruct(Vector3<int> Pos);
    
    // Needs to update; makes sure no above-surface items are rendered
    void SetLayerCutoff(int Cutoff);
    
    // Get the current cutoff
    int GetLayerCutoff();
    
    // Standard render and update functions from GrfxObject
    void Render();
    void Update(float dT);
    
private:
    
    // World depth
    int WorldDepth;
    
    // List of structs (a list per each level)
    List< StructsView_Struct >* Structs;
    
    // World camera angle
    float CameraTheta;
    
    // World cutoff
    int Cutoff;
    
    // World data
    WorldContainer* WorldData;
};

#endif
