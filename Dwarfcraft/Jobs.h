/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Jobs.h/cpp
 Desc: Hands out jobs to a given dwarf.
 
***************************************************************/

// Inclusion guard
#ifndef __JOBS_H__
#define __JOBS_H__

#include "WorldContainer.h"
#include "DesignationsView.h"
#include "ItemsView.h"
#include "DwarfEntity.h"

class Jobs
{
public:
    
    // Constructor and destructor
    Jobs(WorldContainer* MainWorld, DesignationsView* MainDesignations, ItemsView* MainItems);
    ~Jobs();
    
    // Given an entity, give it a job
    void GetJob(DwarfEntity* Dwarf);
    
protected:
    
    // Job specific task management
    bool GetMiningJob(DwarfEntity* Dwarf);
    bool GetFarmerJob(DwarfEntity* Dwarf);
    bool GetCrafterJob(DwarfEntity* Dwarf);
    
private:
    
    // Data handles
    WorldContainer* MainWorld;
    DesignationsView* MainDesignations;
    ItemsView* MainItems;
    
};

// End of inclusion guard
#endif
