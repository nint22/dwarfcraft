/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: VolumeView.h/cpp
 Desc: Manages and renders all major volume selection types and
 associated jobs: building placements, designations, placements,
 and zones. (Originally called designations view)
 
 NOTE: This entire class is thread safe, with designation data
 only being able to be manipulated one at a time. This is done
 so that dwarfs can still manipulate the world data (as that is
 too big and too important to lock) but the instruction generation
 is relatively safe. This is all assuming access to one object,
 and not across multiple objects. This is why the object must
 be handled by address (pointers).
 
***************************************************************/

#ifndef __VOLUMEVIEW_H__
#define __VOLUMEVIEW_H__

#include "WorldContainer.h"
#include "Vector3.h"
#include "List.h"
#include <pthread.h>

// Forward declare structs as needed
struct VolumeTask;
class DwarfEntity;

// Define the job types that exist (closely related to designation types
enum JobType
{
    JobType_Mine,
    JobType_PlaceBlock,     // Building
    JobType_StockpilePos,   // Where we can stockpile
    JobType_ZonePos,        // Where there is a zone
    // Todo: All other job types
};

// A simple "job" object, to handle our task
struct JobTask
{
    // Volume we are working on
    VolumeTask* Volume;
    
    // The sub-job type
    JobType Type;
    
    // The target block we are trying to modify (optional)
    // Note: this may or may not be a block we want to go into, it
    // all depends on the job type and if it is a half step or not
    Vector3<int> TargetBlock;
};

// A task volume structure
struct VolumeTask
{
    /*** Data ***/
    
    // Type and volume of a designation
    UI_RootMenu Category;
    union {
        UI_BuildMenu Building;
        UI_DesignationMenu Designation;
        UI_StockpilesMenu Stockpile;
        UI_ZonesMenu Zone;
    } Type;
    
    // Volume we are working on
    Vector3<int> Origin;
    Vector3<int> Volume;
    
    // A list of jobs that need to be done for this task
    Queue< JobTask* > Jobs;
    Queue< JobTask* > AssignedJobs;
    
    /*** Helper Constructor ***/
    
    VolumeTask(UI_RootMenu Category, int Type, Vector3<int> Origin, Vector3<int> Volume)
    {
        LockInit();
        this->Origin = Origin;
        this->Volume = Volume;
        this->Category = Category;
        if(this->Category == UI_RootMenu_Build)
            this->Type.Building = (UI_BuildMenu)Type;
        else if(this->Category == UI_RootMenu_Designations)
            this->Type.Designation = (UI_DesignationMenu)Type;
        else if(this->Category == UI_RootMenu_Stockpiles)
            this->Type.Stockpile = (UI_StockpilesMenu)Type;
        else if(this->Category == UI_RootMenu_Zones)
            this->Type.Zone = (UI_ZonesMenu)Type;
    }
    
    /*** Lock ***/
    
    // A lock associated with manipulating any of the above data
    pthread_mutex_t Lock;
    
    // Simple init, lock, and unlock (helper functions)
    inline void LockInit() {
        pthread_mutex_init(&Lock, NULL);
    }
    inline void LockData() {
        pthread_mutex_lock(&Lock);
    }
    inline void UnlockData() {
        pthread_mutex_unlock(&Lock);
    }
};

class VolumeView
{
public:
    
    // Standard constructor and destructor
    VolumeView(WorldContainer* MainWorld);
    ~VolumeView();
    
    /*** Manage (add / remove) jobs / volumes ***/
    
    // Add / remove building
    void AddBuilding(UI_BuildMenu Type, Vector3<int> Origin, Vector3<int> Volume);
    void AddDesignation(UI_DesignationMenu Type, Vector3<int> Origin, Vector3<int> Volume);
    void AddStockpile(UI_StockpilesMenu Type, Vector3<int> Origin, Vector3<int> Volume);
    void AddZone(UI_ZonesMenu Type, Vector3<int> Origin, Vector3<int> Volume);
    
    /*** Job Management ***/
    
    // Given an entity, find a job that fits the dwarf's preferences and current world needs
    bool GetJob(DwarfEntity* Dwarf, JobTask* JobOut);
    
    // Unable to complete job
    void ResignJob(JobTask* Job);
    
    // Completed a job
    // Note: this puts the designation to the front of the list as it is confirmed "accesible"
    void CompleteJob(JobTask* Job);
    
private:
    
    // Job specific task management
    bool GetMiningJob(DwarfEntity* Dwarf, JobTask* JobOut);
    bool GetFarmerJob(DwarfEntity* Dwarf, JobTask* JobOut);
    bool GetCrafterJob(DwarfEntity* Dwarf, JobTask* JobOut);
    
    // Returns true if this given block can be accessed by a dwarf
    bool AdjacentAccessible(Vector3<int> Pos);
    
public:
    
    /*** Core Render & Update ***/
    
    // Update and possible regenerate some VBOs
    void Update(float dT);
    
    // Render this Volume (does all translations internally)
    void Render(int LayerCutoff);
    
private:
    
    // Render an volume given a list
    void RenderVolume(List< VolumeTask* >* VolumeList);
    
    // List of each major data and associated lock
    List< VolumeTask* > BuildingList;
    List< VolumeTask* > DesignationList;
    List< VolumeTask* > StockpileList;
    List< VolumeTask* > ZoneList;
    pthread_mutex_t VolumeLock;
    
    // World handle
    WorldContainer* WorldData;
};

#endif
