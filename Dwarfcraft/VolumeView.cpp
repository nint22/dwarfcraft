/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "VolumeView.h"
#include "DwarfEntity.h"

VolumeView::VolumeView(WorldContainer* MainWorld, g2Theme* MainTheme)
{
    // Save world data
    WorldData = MainWorld;
    pthread_mutex_init(&VolumeLock, NULL);
    
    // Get the icon texture info
    MainTheme->GetComponent("IconsList", &IconSrcX, &IconSrcY, &IconSrcW, &IconSrcH, NULL, NULL, &IconTextureID);
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
                Job->Attempts = 0;
                Task->Jobs.Enqueue(Job);
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
                Job->Attempts = 0;
                Task->Jobs.Enqueue(Job);
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
                Job->Attempts = 0;
                Task->Jobs.Enqueue(Job);
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
                Job->Attempts = 0;
                Task->Jobs.Enqueue(Job);
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

bool VolumeView::GetJob(DwarfEntity* Dwarf, JobTask** JobOut)
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
    // What is the job's working volume?
    VolumeTask* Volume = Job->Volume;
    
    // Lock volume
    Volume->LockData();
    
    // Update the job attempt acount
    Job->Attempts++;
    
    // Move this job from the assigned list to unassigned
    int AssignedCount = Volume->AssignedJobs.GetSize();
    for(int i = 0; i < AssignedCount; i++)
    {
        // If we matched the job, pop it off the queue
        JobTask* Task = Volume->AssignedJobs.Dequeue();
        if(Task == Job)
            break;
        Volume->AssignedJobs.Enqueue(Task);
    }
    
    // Put job back into the jobs list
    Volume->Jobs.Enqueue(Job);
    
    // Release lock
    Volume->UnlockData();
}

void VolumeView::CompleteJob(JobTask* Job)
{
    // What is the job's working volume?
    VolumeTask* Volume = Job->Volume;
    
    // Lock volume
    Volume->LockData();
    
    // Move this job from the assigned list to unassigned
    int AssignedCount = Volume->AssignedJobs.GetSize();
    for(int i = 0; i < AssignedCount; i++)
    {
        // If we matched the job, pop it off the queue
        JobTask* Task = Volume->AssignedJobs.Dequeue();
        if(Task == Job)
        {
            delete Task;
            break;
        }
        Volume->AssignedJobs.Enqueue(Task);
    }
    
    // Check though if we are done with the volume
    bool VolumeComplete = Volume->AssignedJobs.GetSize() <= 0 && Volume->Jobs.GetSize() <= 0;
    
    // Release lock
    Volume->UnlockData();
    
    // If volume is empty, release
    if(VolumeComplete)
    {
        // Lock designations
        pthread_mutex_lock(&VolumeLock);
        
        List< VolumeTask* >* List = NULL;
        if(Volume->Category == UI_RootMenu_Build)
            List = &BuildingList;
        else if(Volume->Category == UI_RootMenu_Designations)
            List = &DesignationList;
        else if(Volume->Category == UI_RootMenu_Stockpiles)
            List = &StockpileList;
        else if(Volume->Category == UI_RootMenu_Zones)
            List = &ZoneList;
        
        // Match the volume to release
        int Volumecount = List->GetSize();
        for(int i = 0; i < Volumecount; i++)
        {
            // Matched; remove
            if((*List)[i] == Volume)
            {
                List->Remove(i);
                delete Volume;
                break;
            }
        }
        
        // Unlock designations
        pthread_mutex_unlock(&VolumeLock);
    }
}

bool VolumeView::GetMiningJob(DwarfEntity* Dwarf, JobTask** JobOut)
{
    // Do any of the following jobs in order if possible:
    //   UI_DesignationMenu_Mine
    //   UI_DesignationMenu_Fill
    //   UI_DesignationMenu_Flood
    
    // Best job we have (i.e. the lowest attempt count)
    JobTask* BestJob = NULL;
    
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
                    // Save job if either best job is null OR this job has a lower attempt count
                    if(BestJob == NULL || BestJob->Attempts > Job->Attempts)
                        BestJob = Job;
                }
                
                // Putback
                Volume->Jobs.Enqueue(Job);
            }
        }
    }
    
    // If found job, remove self from jobs queue and add self to assigned queue
    if(BestJob != NULL)
    {
        // Volume we will work on
        VolumeTask* Volume = BestJob->Volume;
        
        // Remove from 
        int JobCount = Volume->Jobs.GetSize();
        for(int JobIndex = 0; JobIndex < JobCount; JobIndex++)
        {
            // Peek
            JobTask* Job = Volume->Jobs.Dequeue();
            
            // If match, pop off from jobs queue
            if(BestJob == Job)
                break;
            
            // Putback
            Volume->Jobs.Enqueue(Job);
        }
        
        // Save self to assigned jobs queue
        BestJob->Volume->AssignedJobs.Enqueue(BestJob);
        *JobOut = BestJob;
    }
    
    // Return true if we found a job
    return BestJob != NULL;
}

bool VolumeView::GetFarmerJob(DwarfEntity* Dwarf, JobTask** JobOut)
{
    //UI_DesignationMenu_Fell,
    //UI_DesignationMenu_Forage,
    //UI_BuildMenu_Farm,          // Simple farm
    return false;
}

bool VolumeView::GetCrafterJob(DwarfEntity* Dwarf, JobTask** JobOut)
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
        dBlock SourceBlock = WorldData->GetBlock(SourcePosition);
        
        Vector3<int> BelowPosition = SourcePosition + Vector3<int>(0, -1, 0);
        if(!WorldData->IsWithinWorld(BelowPosition))
            continue;
        dBlock BelowBlock = WorldData->GetBlock(BelowPosition);
        
        // In air and above a solid block
        if(SourceBlock.GetType() == dBlockType_Air && BelowBlock.IsWhole() && dIsSolid(BelowBlock))
            return true;
        
        // Get the above position
        Vector3<int> AbovePosition = SourcePosition + Vector3<int>(0, 1, 0);
        if(!WorldData->IsWithinWorld(AbovePosition))
            continue;
        dBlock AboveBlock = WorldData->GetBlock(AbovePosition);
        
        // If on a half block, above must be air (j != 3 means we can't be directly above target)
        if(j != 3 && !SourceBlock.IsWhole() && AboveBlock.IsWhole() && AboveBlock.GetType() == dBlockType_Air)
            return true;
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
        
        // Get the texture for this job type
        float tx, ty, tw, th;
        GetIconInfo(GetIconType(Volume), &tx, &ty, &tw, &th);
        
        // Enable and set texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, IconTextureID);
        
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
                        glTexCoord2f(tx, ty);
                        glVertex3f(TilePos.x + 0.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 0.0f);
                        glTexCoord2f(tx + tw, ty);
                        glVertex3f(TilePos.x + 0.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 1.0f);
                        glTexCoord2f(tx + tw, ty + th);
                        glVertex3f(TilePos.x + 1.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 1.0f);
                        glTexCoord2f(tx, ty + th);
                        glVertex3f(TilePos.x + 1.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 0.0f);
                    glEnd();
                }
            }
        }
        
        // Done with texture
        glDisable(GL_TEXTURE_2D);
    }
}

IconType VolumeView::GetIconType(VolumeTask* Volume)
{
    IconType IType = Icon_Cancel; // Default to error
    
    if(Volume->Category == UI_RootMenu_Build)
        IType = IconType(Icon_BuildMenu_Farm + (int)Volume->Type.Building);
    else if(Volume->Category == UI_RootMenu_Designations)
        IType = IconType(Icon_DesignationMenu_Fell + (int)Volume->Type.Designation);
    else if(Volume->Category == UI_RootMenu_Stockpiles)
        IType = IconType(Icon_StockpilesMenu_Rubbish + (int)Volume->Type.Stockpile);
    else if(Volume->Category == UI_RootMenu_Zones)
        IType = IconType(Icon_ZonesMenu_Hall + (int)Volume->Type.Zone);
    
    return IType;
}

void VolumeView::GetIconInfo(IconType Type, float* SrcX, float* SrcY, float* SrcW, float* SrcH)
{
    // Post the size and grid offset
    *SrcX = IconSrcX + float((int)Type % 4) * IconSrcW;
    *SrcY = IconSrcY + float((int)Type / 4) * IconSrcH;
    *SrcW = IconSrcW;
    *SrcH = IconSrcH;
}
