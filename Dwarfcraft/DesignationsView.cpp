/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "DesignationsView.h"
#include "DwarfEntity.h"

void dGetDesignationTexture(DesignationType Type, float* x, float* y, float* width, float* height, GLuint* TerrainID)
{
    // Get the total width and height
    int TextureWidth, TextureHeight, TileSize;
    *TerrainID = dGetTerrainTextureID(&TextureWidth, &TextureHeight, &TileSize);
    
    // Get the appropriate base-face location
    *x = float(dDesignationTexturePos.x * TileSize) / float(TextureWidth);
    *y = float(dDesignationTexturePos.y * TileSize) / float(TextureHeight);
    *width = float(TileSize) / float(TextureWidth);
    *height = float(TileSize) / float(TextureHeight);
    
    // Offset based on tye (firs type is 0)
    *x += *width * int(Type);
}

DesignationsView::DesignationsView(WorldContainer* MainWorld)
{
    // Save world data
    WorldData = MainWorld;
    
    // Initialize lock
    pthread_mutex_init(&DesignationsLock, NULL);
}

DesignationsView::~DesignationsView()
{
    // Nothing to release...
    pthread_mutex_destroy(&DesignationsLock);
}

void DesignationsView::AddDesignation(DesignationType Type, Vector3<int> Origin, Vector3<int> Volume)
{
    // Ignore if volume is empty
    if(Volume.GetLength() <= 0)
        return;
    
    // Struct to insert
    Designation* ToInsert = new Designation();
    ToInsert->Type = Type;
    ToInsert->Origin = Origin;
    ToInsert->Volume = Volume;
    ToInsert->JobsOut = 0;
    
    // If mining, put all non-air blocks as targets to remove
    if(Type == DesignationType_Mine)
    {
        // Always build from top-down
        for(int y = Origin.y + Volume.y - 1; y >= Origin.y; y--)
        for(int x = Origin.x; x < Origin.x + Volume.x; x++)
        for(int z = Origin.z; z < Origin.z + Volume.z; z++)
            if(WorldData->GetBlock(x, y, z).GetType() != dBlockType_Air)
                ToInsert->TaskPositions.Enqueue(Vector3<int>(x, y, z));
    }
    
    // Add to queue
    pthread_mutex_lock(&DesignationsLock);
    int Length = DesignationsList.GetSize();
    DesignationsList.Resize(Length + 1);
    DesignationsList[Length] = ToInsert;
    pthread_mutex_unlock(&DesignationsLock);
}

void DesignationsView::RemoveDesignation(Designation* TargetDesignation)
{
    // Lock while searching
    pthread_mutex_lock(&DesignationsLock);
    
    // Keep cycling through until we find the one we want to remove
    int DesignationsCount = DesignationsList.GetSize();
    for(int i = 0; i < DesignationsCount; i++)
    {
        // If this is the target point, remove
        if(DesignationsList[i] == TargetDesignation)
        {
            delete DesignationsList[i];
            DesignationsList.Remove(i);
            break;
        }
    }
    
    // Done!
    pthread_mutex_unlock(&DesignationsLock);
}

bool DesignationsView::FindDesignation(DesignationType Type, Designation** TargetDesignationOut, Vector3<int>* TargetBlock)
{
    // Lock and copy
    pthread_mutex_lock(&DesignationsLock);
    
    // True if a position was found
    bool Found = false;
    
    // Keep cycling through until we find the one we want to remove
    // Note: We attempt to do designations in order of addition
    int DesignationsCount = DesignationsList.GetSize();
    for(int dIndex = 0; dIndex < DesignationsCount; dIndex++)
    {
        // Pop off to peek
        Designation* Area = DesignationsList[dIndex];
        *TargetDesignationOut = Area;
        
        // If filtered type
        if(Area->Type == Type)
        {
            // For all the blocks left to do access..
            int TaskCount = Area->TaskPositions.GetSize();
            for(int i = 0; i < TaskCount && !Found; i++)
            {
                // Get the task block
                *TargetBlock = Area->TaskPositions.Dequeue();
                
                // Is this position next to air? (This is to attempt to give a possibley accesable block)
                for(int j = 0; j < AdjacentOffsetsCount && !Found; j++)
                {
                    // The air block position
                    Vector3<int> TargetPosition = *TargetBlock + AdjacentOffsets[j];
                    dBlock BlockCheck = WorldData->GetBlock(TargetPosition);
                    
                    // Check if ether an air block or half block
                    // Special exception: if we are above our target, we cannot be in a half block
                    if(WorldData->IsWithinWorld(TargetPosition) && (BlockCheck.GetType() == dBlockType_Air || (!BlockCheck.IsWhole() && j != 0)))
                    {
                        // Found a job, increase the job-out increment
                        Found = true;
                        Area->JobsOut++;
                        printf("Jobs out: %d\n", Area->JobsOut);
                    }
                }
            }
        }
    }
    
    // No need for data access
    pthread_mutex_unlock(&DesignationsLock);
    
    // All done
    return Found;
}

List< Designation* > DesignationsView::GetDesignations()
{
    // Returns a copy
    return DesignationsList;
}

bool DesignationsView::GetJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    // Build a list of the top priority
    Queue<DwarfJobs> TopJobs;
    
    // For each priority order, from high to low
    for(int j = DwarfJobPriority_High; j >= DwarfJobPriority_Low; j--)
    {
        // For each job type
        for(int i = 0; i < DwarfJobsCount; i++)
        {
            if(Dwarf->GetJobPriority()[i] == j)
                TopJobs.Enqueue((DwarfJobs)i);
        }
        
        // If we have something to do, stop
        if(TopJobs.GetSize() > 0)
            break;
    }
    
    // TESTING: Default to just mine
    DwarfJobs TargetJob = DwarfJobs_Miner;
    // Randomly select a job
    /*int Random = rand() % TopJobs.GetSize();
     DwarfJobs TargetJob;
     for(int i = 0; i < Random; i++)
     TopJobs.Dequeue();
     TargetJob = TopJobs.Dequeue();
    */
    
    // Given a job..
    // Note: Each one does a lock internally
    if(TargetJob == DwarfJobs_Miner)
        return GetMiningJob(Dwarf, JobOut);
    
    else if(TargetJob == DwarfJobs_Farmer)
        return GetFarmerJob(Dwarf, JobOut);
    
    else if(TargetJob == DwarfJobs_Crafter)
        return GetCrafterJob(Dwarf, JobOut);
    
    // Undefined
    return false;
}

void DesignationsView::ResignJob(JobTask* Job)
{
    // Failed the job, so let us place it back into the queue
    pthread_mutex_lock(&DesignationsLock);
    Job->TargetDesignation->TaskPositions.Enqueue(Job->TargetBlock);
    Job->TargetDesignation->JobsOut--;
    printf("Jobs out: %d\n", Job->TargetDesignation->JobsOut);
    pthread_mutex_unlock(&DesignationsLock);
}

void DesignationsView::CompleteJob(JobTask* Job)
{
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
}

bool DesignationsView::GetMiningJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    JobOut->Type = JobType_Mine;
    return FindDesignation(DesignationType_Mine, &JobOut->TargetDesignation, &JobOut->TargetBlock);
}

bool DesignationsView::GetFarmerJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    // TODO...
    JobOut->Type = JobType_Mine;
    return FindDesignation(DesignationType_Farm, &JobOut->TargetDesignation, &JobOut->TargetBlock);
}

bool DesignationsView::GetCrafterJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    // TODO...
    JobOut->Type = JobType_Mine;
    return FindDesignation(DesignationType_Hall, &JobOut->TargetDesignation, &JobOut->TargetBlock);
}

void DesignationsView::Update(float dT)
{
    // Do nothing...
}

void DesignationsView::Render(int LayerCutoff, bool Draw)
{
    // Lock to copy date
    pthread_mutex_lock(&DesignationsLock);
    
    // For each designations
    int DesignationsCount = DesignationsList.GetSize();
    for(int dIndex = 0; dIndex < DesignationsCount; dIndex++)
    {
        // Pop off to peek
        Designation* Area = DesignationsList[dIndex];
        
        // Push what we will work on
        glPushMatrix();
        
            // Move and scale as needed (shouldn't it scale first then translate?)
            Vector3<float> VolumeCenter(Area->Origin.x + Area->Volume.x / 2.0f, Area->Origin.y + Area->Volume.y / 2.0f, Area->Origin.z + Area->Volume.z / 2.0f);
            glTranslatef(VolumeCenter.x, VolumeCenter.y, VolumeCenter.z);
            glScalef(Area->Volume.x + 0.2f, Area->Volume.y + 0.2f, Area->Volume.z + 0.2f);
            
            // Draw cube
            glColor3f(0.2f, 0.2f, 0.2f);
            glLineWidth(2.0f);
            glutWireCube(1.0f);
        
        // Pop this localized coordinate
        glPopMatrix();
        
        // Get texture info
        float srcx, srcy, srcwidth, srcheight; GLuint TextureID;
        dGetDesignationTexture(Area->Type, &srcx, &srcy, &srcwidth, &srcheight, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        
        glEnable(GL_TEXTURE_2D);
        
        static float phase = 0.0f;
        phase += 0.1f;
        glColor4f(1, 1, 1, 0.9f + 0.1f * sin(phase));
        
        // Copy all positions we are editing
        Queue< Vector3<int> > Tiles = Area->TaskPositions;
        while(!Tiles.IsEmpty())
        {
            // Get the position
            Vector3<int> TilePos = Tiles.Dequeue();
            
            // Only render if we are right below air
            if(WorldData->IsWithinWorld(TilePos + Vector3<int>(0, 1, 0)) && WorldData->GetBlock(TilePos + Vector3<int>(0, 1, 0)).GetType() == dBlockType_Air)
            {
                // Move down 0.5f if half block
                float VerticalOffset = 0.0f;
                if(!WorldData->GetBlock(TilePos).IsWhole())
                    VerticalOffset = -0.5f;
                
                // Do a tiny oscilating offset
                VerticalOffset += 0.01f + 0.005f * sin(phase * 0.5f + TilePos.z) + 0.01f + 0.005f * cos(phase * 0.5f + TilePos.x);
                
                // Render a mining tile
                // Note the slight shift upwards because we want to render it ABOVE a block
                glBegin(GL_QUADS);
                    glTexCoord2f(srcx + srcwidth, srcy); glVertex3f(TilePos.x + 0.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 0.0f);
                    glTexCoord2f(srcx, srcy); glVertex3f(TilePos.x + 0.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 1.0f);
                    glTexCoord2f(srcx, srcy + srcheight); glVertex3f(TilePos.x + 1.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 1.0f);
                    glTexCoord2f(srcx + srcwidth, srcy + srcheight); glVertex3f(TilePos.x + 1.0f, TilePos.y + 1.01f + VerticalOffset, TilePos.z + 0.0f);
                glEnd();
            }
        }
        
        glDisable(GL_TEXTURE_2D);
    }
    
    // Done with data lock
    pthread_mutex_unlock(&DesignationsLock);
}
