/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: DwarfEntity.h/cpp
 Desc: The dwarf AI and rendering controlling entity.
 
 Current design comes from the design doc:
 https://docs.google.com/document/d/18acyyFY4rZzw6XeuuVuwhH-uE8oR8RuolIMFHP3YHkE/edit?hl=en_US
 
 All dwarfs follow the below simple logical flow chart. Based on
 what is near the dwarf, as well as its own status, and current
 commands the user has issues, it may do one of many different things.
 
***************************************************************/

// Inclusion guard
#ifndef __DWARFENTITY_H__
#define __DWARFENTITY_H__

#include "Entity.h"

// Define the three major job types (Gaints experiance)
static const int DwarfJobsCount = 3;
enum DwarfJobs
{
    DwarfJobs_Farmer,
    DwarfJobs_Miner,
    DwarfJobs_Crafter,
};

// Define the secondary class (temporary jobs; does not gain exp.)
static const int DwarfMinorJobsCount = 3;
enum DwarfMinorJobs
{
    DwarfMinorJobs_Combat,
    DwarfMinorJobs_Medic,
    DwarfMinorJobs_Clerk,
};

// Priority of each job or minor job (ranges from low, mid, high)
static const int DwarfJobPriorityCount = 3;
enum DwarfJobPriority
{
    DwarfJobPriority_Low,
    DwarfJobPriority_Medium,
    DwarfJobPriority_High,
};

// Dwarf rank
static const int DwarfRankCount = 13;
enum DwarfRank
{
    DwarfRank_Child,
    DwarfRank_Conscript,
    DwarfRank_Private,
    DwarfRank_Corporal,
    DwarfRank_Sergant,
    DwarfRank_StaffSergant,
    DwarfRank_MasterSergant,
    DwarfRank_MasterChief,
    DwarfRank_Captain,
    DwarfRank_Major,
    DwarfRank_Colonel,
    DwarfRank_General,
    DwarfRank_King,
};

static const char DwarfRankNames[DwarfRankCount][32] =
{
    "Child",
    "Conscript",
    "Private",
    "Corporal",
    "Sergant",
    "Staff Sergant",
    "Master Sergant",
    "Master Chief",
    "Captain",
    "Major",
    "Colonel",
    "General",
    "King",
};

class DwarfEntity : public Entity
{
public:
    
    // Constructor and destructor
    DwarfEntity(const char* ConfigName);
    ~DwarfEntity();
    
    // Explicitly set the armor set of this dwarf
    void SetArmor(dItem Chest, dItem Legs);
    
    // Custom preview-drawing function so that we can render all armor & tools
    void RenderPreview(int x, int y, int width, int height);
    
    /*** Dwarf Status ***/
    
    // Overloaded; Get the max health count of the dwarf
    int GetMaxHealth();
    
    // Get the happiness scale
    float GetHappiness();
    
    // Get the fatigue scale
    float GetFatigue();
    
    // Get current breath
    float GetBreath();
    
    // Get max breath
    float GetMaxBreath();
    
    // Get hunger
    float GetHunger();
    
    // Get thirst
    float GetThirst();
    
    // Get name (overloaded from Entity)
    const char* GetName();
    
    // Get age
    int GetAge();
    
    // Get gender; true if male, false if female
    bool GetGender();
    
    // Get level
    int GetLevel();
    
    // Get total experiance
    int GetExp();
    
    // Get rank
    DwarfRank GetRank();
    
    /*** Job preferences ***/
    
    // Get job priority array
    DwarfJobPriority* GetJobPriority();
    
    // Get the minor job priority
    DwarfJobPriority* GetMinorJobPriority();
    
protected:
    
    // Update object
    void Update(float dT);
    
    // Custom drawing function so we render the armor on-top
    void Render();
    
private:
    
    // Render the target of where we are going to
    void RenderTargetPath();
    
    // Threaded instruction computation function
    static void* ComputeTask(void* data);
    
    // Is the thread currently running? (Thread safe)
    bool ThreadRunning();
    
    // Do we have a job? (Thread safe)
    bool HasJob();
    
    // The thread handle and data mutex
    bool ThreadRunningFlag;
    pthread_t ThreadHandle;
    pthread_mutex_t ThreadMutex;
    
    // Current job for the dwarf
    JobTask Job;
    bool HasJobFlag;
    
    // Active job path
    Stack<Vector3<int> > JobPath;
    
    /*** Dwarf Properties ***/
    
    // Job level & exp
    int JobExperiance[DwarfJobsCount];
    
    // Job preferences
    DwarfJobPriority MainJobs[DwarfJobsCount];
    DwarfJobPriority MinorJobs[DwarfMinorJobsCount];
    
    // Gender
    bool IsMale;
    
    // Age
    int Age;
    
    // Rank (in the colony)
    DwarfRank Rank;
    
    // Happiness scale
    float Happiness;
    
    // Tired timer / scale
    float Fatigue;
    
    // Breath time (10 seconds underwater until death)
    float BreathTime;
    
    // Hunger & thirst (scale from 0 [damage dealing] and 
    float Hunger, Thirst;
    
    // Inventory / item ID (can only hold up to two items)
    // Note this is a struct that contains info like durability, quality, etc.
    dItem Items[2];
    
    // Armor set (chest, feet for now)
    dItem Armor[2];
};

// End of inclusion guard
#endif
