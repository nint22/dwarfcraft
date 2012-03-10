/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: DesignationsView.h/cpp
 Desc: Manages all game designations for the givn map. This
 is generally a container of all designations for a given world.
 
 NOTE: This entire class is thread safe, with designation data
 only being able to be manipulated one at a time. This is done
 so that dwarfs can still manipulate the world data (as that is
 too big and too important to lock) but the instruction generation
 is relatively safe.
 
***************************************************************/

#ifndef __DESIGNATIONSVIEW_H__
#define __DESIGNATIONSVIEW_H__

#include "WorldContainer.h"
#include "Vector3.h"
#include "Queue.h"
#include <pthread.h>

// Necesary forward declarations
class DwarfEntity;
class Designation;

// A simple "job" object, to handle our task
struct JobTask
{
    // The designation volume we are working on
    Designation* TargetDesignation;
    
    // The position we want to move the dwarf to
    Vector3<int> TargetPosition;
    
    // The target block we are to modify (optional)
    Vector3<int> TargetBlock;
};

// Total number of designation groups
static const int DesignationGroupCount = 4;

// Enumeration of designation groups
enum DesignationGroup
{
    DesignationGroup_Construct,
    DesignationGroup_Storage,
    DesignationGroup_Collect,
    DesignationGroup_Military,
};

// 16 total designations
static const int DesignationCount = 16;

// Enumeration of designations
enum DesignationType
{
    // Construction
    DesignationType_Mine = 0,
    DesignationType_Fill,
    DesignationType_Flood,
    
    // Storage
    DesignationType_Rubbish,
    DesignationType_Food,
    DesignationType_Crafted,
    DesignationType_RawResources, // Wood, stones, etc.
    DesignationType_Ingots,
    DesignationType_Grave,
    
    // Collection
    DesignationType_Farm,
    DesignationType_Wood,
    DesignationType_Forage,
    
    // Military flags
    DesignationType_Protect,
    DesignationType_Barracks,
    DesignationType_Hall,
    DesignationType_Armory,
};

// Human readable names
static const char DesignationNames[DesignationCount][32] =
{
    "Mine",
    "Fill",
    "Flood",
    "Rubbish",
    "Food",
    "Crafted",
    "RawResources",
    "Ingots",
    "Grave",
    "Farm",
    "Wood",
    "Forage",
    "Protect",
    "Barracks",
    "Hall",
    "Armory",
};

// A designation data type
struct Designation
{
    DesignationType Type;
    Vector3<int> Origin;
    Vector3<int> Volume;
    
    // A list of positions in the volume that needs to be processed
    Queue< Vector3<int> > TaskPositions;
};

// Returns the texture coordinates of a given designation; similar to the "dGetBlockTexture(...)" function
void dGetDesignationTexture(DesignationType Type, float* x, float* y, float* width, float* height, GLuint* TerrainID);

class DesignationsView
{
public:
    
    // Standard constructor and destructor
    DesignationsView(WorldContainer* MainWorld);
    ~DesignationsView();
    
    /*** Designation Management ***/
    
    // Add a new designation (takes any two points; will self-organize for origin and volume)
    void AddDesignation(DesignationType Type, Vector3<int> Origin, Vector3<int> Volume);
    
    // Remove a designation that contains the given point
    void RemoveDesignation(Vector3<int> Point);
    
    // Returns a position within the designation that is adjacent to an air block, and thus assumed possible for access
    // The position where the dwarf should go into is "TargetPosition" while "TargetBlock" is what is trying to be manipulated
    bool FindDesignation(DesignationType Type, Designation** TargetDesignationOut, Vector3<int>* TargetPosition, Vector3<int>* TargetBlock);
    
    // Get a copy (not reference) of all designations
    Queue< Designation* > GetDesignations();
    
    /*** Job Management ***/
    
    // Given an entity, find a job that fits the dwarf's preferences and current world needs
    bool GetJob(DwarfEntity* Dwarf, JobTask* JobOut);
    
    // Push back the job that was given to this dwarf
    void PutBackJob(JobTask* JobOut);
    
private:
    
    // Job specific task management
    bool GetMiningJob(DwarfEntity* Dwarf, JobTask* JobOut);
    bool GetFarmerJob(DwarfEntity* Dwarf, JobTask* JobOut);
    bool GetCrafterJob(DwarfEntity* Dwarf, JobTask* JobOut);
    
public:
    
    /*** Core Render & Update ***/
    
    // Update and possible regenerate some VBOs
    void Update(float dT);
    
    // Render this Volume (does all translations internally)
    void Render(int LayerCutoff, bool Draw = true);
    
private:
    
    // List of all designations
    Queue< Designation* > DesignationsQueue;
    
    // World handle
    WorldContainer* WorldData;
    
    // Designations data lock (to help us be thread safe)
    static pthread_mutex_t DesignationsLock;
    static bool DesignationsLock_Init;
};

#endif
