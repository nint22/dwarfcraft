/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: g2Entity.h/cpp
 Desc: A panal that centers on-screen that describes the selected
 entity. It is a portrait-shape with variable details depending
 on what is selected.
 
***************************************************************/

// Inclusion guard
#ifndef __G2ENTITYCONTROLLER_H__
#define __G2ENTITYCONTROLLER_H__

#include <Glui2/glui2.h>
#include "Entity.h"
#include "DwarfEntity.h"

class g2EntityController : public g2Panel
{
public:
    
    // Standard constructor / destructor
    g2EntityController(g2Controller* Owner, g2Theme* MainTheme);
    ~g2EntityController();
    
    /*** Set on-screen properties ***/
    
    // Set all info as if a dwarf
    void SetDwarf(DwarfEntity* Entity);
    
    // Get current dwarf that we have selected
    DwarfEntity* GetDwarf();
    
protected:
    
    // Render the controller
    void Render(int x, int y);
    
    // Update the controller
    void Update(float dT);
    
private:
    
    /*** Helper Functions ***/
    
    // Set the health string
    void SetHealth(int current, int max);
    
    // Set the happiness string
    void SetHappiness(float Happiness, float Fatigue);
    
    // Set the breath / hunger / thirst factors (hunger and thirst are scales, breath relates to max breath)
    void SetBreathHungerThirst(float Breath, float MaxBreath, float Hunger, float Thirst);
    
    // Set the task string
    void SetTask(const char* TaskName);
    
    // Set the rank string
    void SetRank(const char* RankName);
    
    // Set the level and experiance of the dwarf
    void SetLevelExp(int Level, int Exp);
    
    // Set the name of the dwarf
    void SetName(const char* Name);
    
    // Set the age and gender (true if male, false if female)
    void SetAgeGender(int Age, bool IsMale);
    
    // Set the jobs preferences
    void SetJobs(DwarfJobs Job, DwarfMinorJobs MinorJob);
    
    /*** Callback Functions ***/
    
    // Called when the main work pref. changes
    static void WorkPrefSet(g2Controller* Caller);
    
    // Called when the sub work pref. changes
    static void SubWorkPrefSet(g2Controller* Caller);
    
    // Called when path flag changes
    static void PathToggle(g2Controller* Caller);
    
    /*** Form Controllers ***/
    
    // Name
    g2Label* NameText;
    
    // Gender & age
    g2Label* GenderAgeText;
    
    // Job level and experiance string
    g2Label* JobText;
    
    // Colony rank
    g2Label* RankText;
    
    // Current / max health
    g2Label* HealthText;
    
    // Happiness and fatigue
    g2Label* HappinessText;
    
    // Breath, hunger, thirst
    g2Label* BreathHungerThirstText;
    
    // Current task
    g2Label* CurrentTaskText;
    
    // Work preference label
    g2Label* WorkPreferenceText;
    
    // Preference selector
    g2RadioGroup* WorkPreferenceGroup;
    g2RadioGroup* SecondPreferenceGroup;
    
    // Show path or not
    g2CheckBox* ShowPath;
    
    // On-screen elements to render
    Entity* SelectedEntity;
};

#endif
