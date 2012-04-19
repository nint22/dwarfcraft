/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Entities.h/cpp
 Desc: Manages all game entities, as well as entity-entity
 interaction. Is a GrfxObject and thus expects to manage
 all rendering internally. Though children aren't derivatives
 from GrfxObject, just implement the Entity class. This class
 can render the paths of all entities if set via "RenderPath(...)"
 
***************************************************************/

#ifndef __ENTITIES_H__
#define __ENTITIES_H__

#include "GrfxObject.h"
#include "WorldContainer.h"
#include "VolumeView.h"
#include "Entity.h"
#include "Queue.h"

class Entities
{
public:
    
    // Standard constructor and destructor
    Entities(WorldContainer* MainWorld, VolumeView* MainDesignations, ItemsView* MainItems);
    ~Entities();
    
    // Add a new entity; gives the main world reference
    void AddEntity(Entity* NewEntity);
    
    // Get a copy (not reference) of all entities
    Queue< Entity* > GetEntities();
    
    // Render (on or off) the entities path
    void SetRenderingPath(bool Set);
    
    // Get the current render path state
    bool GetRenderingPath();
    
    // Give a ray, attempt to find any entities that intersects it (closest to the ray origin is returned)
    // Returns null if not found
    Entity* IntersectEntities(Vector3<float> RayPos, Vector3<float> RayDir, int CutoffLayer);
    
    // Update all entities
    void Update(float dT);
    
    // Render all entities
    void Render(int LayerCutoff, float CameraAngle);
    
private:
    
    // List of all entities
    List< Entity* > EntitiesList;
    
    // World data handle
    WorldContainer* MainWorld;
    
    // Designations handle
    VolumeView* MainDesignations;
    
    // Items handle
    ItemsView* MainItems;
    
    // Camera's current angle
    float Theta;
    
    // Cutoff layer
    int LayerCutoff;
    
    // Render paths if true
    bool RenderablePath;
};

#endif
