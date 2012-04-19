/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "VolumeView.h"
#include "DwarfEntity.h"

VolumeView::VolumeView(WorldContainer* MainWorld)
{
    // Save world data
    WorldData = MainWorld;
    pthread_mutex_init(&VolumeLock, NULL);
}

VolumeView::~VolumeView()
{
    // Drop lock
    pthread_mutex_destroy(&VolumeLock);
}

void VolumeView::AddBuilding(UI_BuildMenu Type, Vector3<int> Origin, Vector3<int> Volume)
{
    // Allocate
    VolumeTask* Task = new VolumeTask(UI_RootMenu_Build, (int)Type, Origin, Volume);
    
    // Create all the jobs
    for(int y = Origin.y; y < Origin.y + Volume.y; y++)
    for(int x = Origin.x; x < Origin.x + Volume.x; x++)
    for(int z = Origin.z; z < Origin.z + Volume.z; z++)
    {
        // Buildings may only exist above on solid ground
        if(WorldData->IsWithinWorld(x, y - 1, z) && WorldData->IsWithinWorld(x, y, z))
        {
            dBlock BaseBlock = WorldData->GetBlock(x, y - 1, z);
            dBlock TopBlock = WorldData->GetBlock(x, y, z);
            
            // Make the job
            if(BaseBlock.IsWhole() && dIsSolid(BaseBlock) && TopBlock.GetType() == dBlockType_Air)
            {
                JobTask* Job = new JobTask();
                Job->Volume = Task;
                Job->Type = JobType_PlaceBlock; // TODO!!!
                Job->TargetBlock = Vector3<int>(x, y, z);
            }
        }
    }
    
    // Add to list if there is a job to do
    if(Task->Jobs.GetSize() > 0)
    {
        pthread_mutex_lock(&VolumeLock);
            int EndIndex = BuildingList.GetSize();
            BuildingList.Resize(EndIndex + 1);
            BuildingList[EndIndex] = Task;
        pthread_mutex_unlock(&VolumeLock);
    }
    // Else, release
    else
        delete Task;
}

void VolumeView::AddDesignation(UI_DesignationMenu Type, Vector3<int> Origin, Vector3<int> Volume)
{
    // Allocate
    VolumeTask* Task = new VolumeTask(UI_RootMenu_Designations, (int)Type, Origin, Volume);
    
    // Create all the jobs
    for(int y = Origin.y; y < Origin.y + Volume.y; y++)
    for(int x = Origin.x; x < Origin.x + Volume.x; x++)
    for(int z = Origin.z; z < Origin.z + Volume.z; z++)
    {
        // Must be within world
        if(WorldData->IsWithinWorld(x, y, z))
        {
            // Get block
            dBlock Block = WorldData->GetBlock(x, y, z);
            
            // Mine out non-air or fill / flood air
            if((Type == UI_DesignationMenu_Mine && Block.GetType() != dBlockType_Air) ||
               ((Type == UI_DesignationMenu_Fill || Type == UI_DesignationMenu_Flood) && Block.GetType() == dBlockType_Air))
            {
                JobTask* Job = new JobTask();
                Job->Volume = Task;
                Job->Type = JobType_Mine; // TODO
                Job->TargetBlock = Vector3<int>(x, y, z);
            }
        }
    }
    
    // Add to list if there is a job to do
    if(Task->Jobs.GetSize() > 0)
    {
        pthread_mutex_lock(&VolumeLock);
            int EndIndex = DesignationList.GetSize();
            DesignationList.Resize(EndIndex + 1);
            DesignationList[EndIndex] = Task;
        pthread_mutex_unlock(&VolumeLock);
    }
    // Else, release
    else
        delete Task;
}

void VolumeView::AddStockpile(UI_StockpilesMenu Type, Vector3<int> Origin, Vector3<int> Volume)
{
    // Allocate
    VolumeTask* Task = new VolumeTask(UI_RootMenu_Stockpiles, (int)Type, Origin, Volume);
    
    // Create all the jobs
    for(int y = Origin.y; y < Origin.y + Volume.y; y++)
    for(int x = Origin.x; x < Origin.x + Volume.x; x++)
    for(int z = Origin.z; z < Origin.z + Volume.z; z++)
    {
        // Buildings may only exist above on solid ground
        if(WorldData->IsWithinWorld(x, y - 1, z) && WorldData->IsWithinWorld(x, y, z))
        {
            dBlock BaseBlock = WorldData->GetBlock(x, y - 1, z);
            dBlock TopBlock = WorldData->GetBlock(x, y, z);
            
            // Make the job
            if(BaseBlock.IsWhole() && dIsSolid(BaseBlock) && TopBlock.GetType() == dBlockType_Air)
            {
                JobTask* Job = new JobTask();
                Job->Volume = Task;
                Job->Type = JobType_StockpilePos; // TODO!!!
                Job->TargetBlock = Vector3<int>(x, y, z);
            }
        }
    }
    
    // Add to list if there is a job to do
    if(Task->Jobs.GetSize() > 0)
    {
        pthread_mutex_lock(&VolumeLock);
            int EndIndex = StockpileList.GetSize();
            StockpileList.Resize(EndIndex + 1);
            StockpileList[EndIndex] = Task;
        pthread_mutex_unlock(&VolumeLock);
    }
    // Else, release
    else
        delete Task;
}

void VolumeView::AddZone(UI_ZonesMenu Type, Vector3<int> Origin, Vector3<int> Volume)
{
    // Allocate
    VolumeTask* Task = new VolumeTask(UI_RootMenu_Stockpiles, (int)Type, Origin, Volume);
    
    // Create all the jobs
    for(int y = Origin.y; y < Origin.y + Volume.y; y++)
    for(int x = Origin.x; x < Origin.x + Volume.x; x++)
    for(int z = Origin.z; z < Origin.z + Volume.z; z++)
    {
        // Buildings may only exist above on solid ground
        if(WorldData->IsWithinWorld(x, y - 1, z) && WorldData->IsWithinWorld(x, y, z))
        {
            dBlock BaseBlock = WorldData->GetBlock(x, y - 1, z);
            dBlock TopBlock = WorldData->GetBlock(x, y, z);
            
            // Make the job
            if(BaseBlock.IsWhole() && dIsSolid(BaseBlock) && TopBlock.GetType() == dBlockType_Air)
            {
                JobTask* Job = new JobTask();
                Job->Volume = Task;
                Job->Type = JobType_ZonePos; // TODO!!!
                Job->TargetBlock = Vector3<int>(x, y, z);
            }
        }
    }
    
    // Add to list if there is a job to do
    if(Task->Jobs.GetSize() > 0)
    {
        pthread_mutex_lock(&VolumeLock);
            int EndIndex = ZoneList.GetSize();
            ZoneList.Resize(EndIndex + 1);
            ZoneList[EndIndex] = Task;
        pthread_mutex_unlock(&VolumeLock);
    }
    // Else, release
    else
        delete Task;
}

bool VolumeView::GetJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    // Build a list of highest preference to lowesr preference
    Queue<DwarfJobs> JobPriority;
    
    // For each priority order, from high to low
    for(int j = DwarfJobPriority_High; j >= DwarfJobPriority_Low; j--)
    {
        // For each job type
        for(int i = 0; i < DwarfJobsCount; i++)
        {
            if(Dwarf->GetJobPriority()[i] == j)
                JobPriority.Enqueue((DwarfJobs)i);
        }
    }
    
    // Lock since we are going to read list data
    pthread_mutex_lock(&VolumeLock);
    
    // What does this dwarf want to do?
    bool GotJob = false;
    while(!JobPriority.IsEmpty() && !GotJob)
    {
        // Get job
        DwarfJobs Job = JobPriority.Dequeue();
        
        if(Job == DwarfJobs_Farmer)
            GotJob = GetFarmerJob(Dwarf, JobOut);
        else if(Job == DwarfJobs_Miner)
            GotJob = GetMiningJob(Dwarf, JobOut);
        else if(Job == DwarfJobs_Crafter)
            GotJob = GetCrafterJob(Dwarf, JobOut);
    }
    
    // Done working
    pthread_mutex_unlock(&VolumeLock);
    
    // Returns true if we ever found a job
    return GotJob;
}

void VolumeView::ResignJob(JobTask* Job)
{
    /*
    // Failed the job, so let us place it back into the queue
    pthread_mutex_lock(&DesignationsLock);
    Job->TargetDesignation->TaskPositions.Enqueue(Job->TargetBlock);
    Job->TargetDesignation->JobsOut--;
    printf("Jobs out: %d\n", Job->TargetDesignation->JobsOut);
    pthread_mutex_unlock(&DesignationsLock);
    */
}

void VolumeView::CompleteJob(JobTask* Job)
{
    /*
    // No need to do anything, the item is off the queue already
    
    // If this was the last item, delete the designation
    pthread_mutex_lock(&DesignationsLock);
    Job->TargetDesignation->JobsOut--;
    printf("Jobs out: %d\n", Job->TargetDesignation->JobsOut);
    int JobsOut = Job->TargetDesignation->JobsOut;
    bool IsEmpty = Job->TargetDesignation->TaskPositions.IsEmpty();
    pthread_mutex_unlock(&DesignationsLock);
    
    // Perform delete outside of lock, as we actually already have a lock
    if(IsEmpty && JobsOut == 0)
        RemoveDesignation(Job->TargetDesignation);
    */
}

bool VolumeView::GetMiningJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    // Do any of the following jobs in order if possible:
    //   UI_DesignationMenu_Mine
    //   UI_DesignationMenu_Fill
    //   UI_DesignationMenu_Flood
    
    // For each designation
    int DesignationCount = DesignationList.GetSize();
    for(int DesignationIndex = 0; DesignationIndex < DesignationCount; DesignationIndex++)
    {
        // Look at this designation: is it what we want?
        VolumeTask* Volume = DesignationList[DesignationIndex];
        UI_DesignationMenu Type = Volume->Type.Designation;
        if(Volume->Category == UI_RootMenu_Designations && (Type == UI_DesignationMenu_Mine || Type == UI_DesignationMenu_Fill || Type == UI_DesignationMenu_Flood))
        {
            // For each job
            int JobCount = Volume->Jobs.GetSize();
            for(int JobIndex = 0; JobIndex < JobCount; JobIndex++)
            {
                // Peek
                JobTask* Job = Volume->Jobs.Dequeue();
                
                // See if this job position is trivial to reach (i.e. adjacently accessible)
                if(AdjacentAccessible(Job->TargetBlock))
                {
                    // Save job and push into the "checked out jobs list"
                    Volume->AssignedJobs.Enqueue(Job);
                    *JobOut = *Job;
                    return true;
                }
                
                // Putback
                Volume->Jobs.Enqueue(Job);
            }
        }
    }
    
    return false;
}

bool VolumeView::GetFarmerJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    //UI_DesignationMenu_Fell,
    //UI_DesignationMenu_Forage,
    //UI_BuildMenu_Farm,          // Simple farm
    return false;
}

bool VolumeView::GetCrafterJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    //UI_BuildMenu_Architecture,  // Stairs, floors, walls, etc.
    //UI_BuildMenu_Workshops,     // Masonry, woodshop, etc.
    //UI_BuildMenu_Furniture,     // Chairs, doors, etc.
    //UI_BuildMenu_Mechanical,    // Levers, gates, etc.
    return false;
}

bool VolumeView::AdjacentAccessible(Vector3<int> Pos)
{
    // For each possible offset origin
    for(int j = 0; j < AdjacentOffsetsCount; j++)
    {
        // Where the dwarf will be and the block below
        Vector3<int> SourcePosition = Pos + AdjacentOffsets[j];
        Vector3<int> BasePosition = Pos + AdjacentOffsets[j];
        dBlock BlockCheck = WorldData->GetBlock(SourcePosition);
        
        // If the surce is half a block, we can reach to the target
        if(WorldData->IsWithinWorld(SourcePosition) && BlockCheck.GetType() != dBlockType_Air && !BlockCheck.IsWhole())
        {
            // Special exception: we can't mine from directly ontop to the block below if it is a half block
            // i.e. if the source is directly above the target, then the source block must be air
            if(j != 3)
                return true;
        }
        
        // Check if either the source position an air block ontop of a solid block *or* is just a half block
        // Special exception: if we are above our target, we cannot be in a half block
        else if(WorldData->IsWithinWorld(BasePosition))
        {
            dBlock BaseCheck = WorldData->GetBlock(BasePosition);
            if(BlockCheck.GetType() == dBlockType_Air && BaseCheck.GetType() != dBlockType_Air && BaseCheck.IsWhole())
                return true;
        }
    }
    
    return false;
}

void VolumeView::Update(float dT)
{
    // Do nothing...
}

void VolumeView::Render(int LayerCutoff)
{
    // Lock to copy date
    pthread_mutex_lock(&VolumeLock);
    
    // Render the given list of VolumeTasks
    RenderVolume(&BuildingList);
    RenderVolume(&DesignationList);
    RenderVolume(&StockpileList);
    RenderVolume(&ZoneList);
    
    // Done with data lock
    pthread_mutex_unlock(&VolumeLock);
}

void VolumeView::RenderVolume(List< VolumeTask* >* VolumeList)
{
    // For each job
    int VolumeCount = VolumeList->GetSize();
    for(int VolumeIndex = 0; VolumeIndex < VolumeCount; VolumeIndex++)
    {
        // Get volume
        VolumeTask* Volume = (*VolumeList)[VolumeIndex];
        
        // Push what we will work on
        glPushMatrix();
        
            // Move and scale as needed (shouldn't it scale first then translate?)
            Vector3<float> VolumeCenter(Volume->Origin.x + Volume->Volume.x / 2.0f, Volume->Origin.y + Volume->Volume.y / 2.0f, Volume->Origin.z + Volume->Volume.z / 2.0f);
            glTranslatef(VolumeCenter.x, VolumeCenter.y, VolumeCenter.z);
            glScalef(Volume->Volume.x + 0.2f, Volume->Volume.y + 0.2f, Volume->Volume.z + 0.2f);
            
            // Draw cube
            glColor3f(0.2f, 0.2f, 0.2f);
            glLineWidth(2.0f);
            glutWireCube(1.0f);
        
        // Pop this localized coordinate
        glPopMatrix();
        
        // Render each job queue
        for(int QueueIndex = 0; QueueIndex < 2; QueueIndex++)
        {
            // Set the global tile color
            static float phase = 0.0f;
            phase += 0.1f;
            
            glColor4f(1, 1, 1, 0.9f + 0.1f * sin(phase));
            if(QueueIndex == 1)
                glColor4f(1, 0.5f, 0.5f, 0.9f + 0.1f * cos(phase));
            
            // Tasks we need to render
            Queue< JobTask* > JobList = Volume->Jobs;
            if(QueueIndex == 1)
                JobList = Volume->AssignedJobs;
            
            // Copy all positions we are editing
            while(!JobList.IsEmpty())
            {
                // Get the position
                Vector3<int> TilePos = JobList.Dequeue()->TargetBlock;
                
                // Only render if we are right below air
                if(WorldData->IsWithinWorld(TilePos + Vector3<int>(0, 1, 0)) && WorldData->GetBlock(TilePos + Vector3<int>(0, 1, 0)).GetType() == dBlockType_Air)
                {
                    // Move down 0.5f if half block
                    float VerticalOffset = 0.0f;
                    if(!WorldData->GetBlock(TilePos).IsWhole())
                        VerticalOffset = -0.5f;
                    
                    // Do a tiny oscilating offset
                    VerticalOffset += 0.02f + 0.005f * sin(phase * 0.5f + TilePos.z) + 0.005f * cos(phase * 0.5f + TilePos.x);
                    
                    // Render a mining tile
                    // Note the slight shift upwards because we want to render it ABOVE a block
                    glBegin(GL_QUADS);
                        glVertex3f(TilePos.x + 0.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 0.0f);
                        glVertex3f(TilePos.x + 0.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 1.0f);
                        glVertex3f(TilePos.x + 1.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 1.0f);
                        glVertex3f(TilePos.x + 1.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 0.0f);
                    glEnd();
                }
            }
        }
    }
}
