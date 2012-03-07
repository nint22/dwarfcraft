/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "Jobs.h"

Jobs::Jobs(WorldContainer* MainWorld, DesignationsView* MainDesignations, ItemsView* MainItems)
{
    this->MainWorld = MainWorld;
    this->MainDesignations = MainDesignations;
    this->MainItems = MainItems;
}

Jobs::~Jobs()
{
    
}

void Jobs::GetJob(DwarfEntity* Dwarf)
{
    // The target job we will attempt to generate for the given dwarf's priority
    DwarfJobs TargetJob = DwarfJobs_Miner;
    
    // For each priority order, from high to low
    for(int j = DwarfJobPriority_High; j >= DwarfJobPriority_Low; j--)
    {
        // For each job type
        for(int i = 0; i < DwarfJobsCount; i++)
        {
            if(Dwarf->GetJobPriority()[i] == j)
                TargetJob = (DwarfJobs)j;
        }
    }
    
    // Given a job..
    if(TargetJob == DwarfJobs_Miner)
        GetMiningJob(Dwarf);
    
    else if(TargetJob == DwarfJobs_Farmer)
        GetFarmerJob(Dwarf);
    
    else if(TargetJob == DwarfJobs_Crafter)
        GetCrafterJob(Dwarf);
}

bool Jobs::GetMiningJob(DwarfEntity* Dwarf)
{
    // Right now, JUST do mining; later: filling and flooding
    
    // Get a list of the best possible designations; sorted in order of street distance
    Queue< std::pair< Vector3<int>, Vector3<int> > > MiningPosList = MainDesignations->FindDesignation(DesignationType_Mine, Dwarf->GetPositionBlock());
    
    // Return true if we found a valid path
    return MiningPosList.GetSize() > 0;
}

bool Jobs::GetFarmerJob(DwarfEntity* Dwarf)
{
    // Right now, JUST do mining; later: filling and flooding
    
    // Get a list of the best possible designations; sorted in order of street distance
    Queue< std::pair< Vector3<int>, Vector3<int> > > FarmingPosList = MainDesignations->FindDesignation(DesignationType_Farm, Dwarf->GetPositionBlock());
    
    // Return true if we found a valid path
    return FarmingPosList.GetSize() > 0;
}

bool Jobs::GetCrafterJob(DwarfEntity* Dwarf)
{
    // Get a list of all the crafting tables, etc..
    
    // Not yet implemented
    return false;
}
