/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "DesignationsView.h"
#include "DwarfEntity.h"

// Since this is a static var, we need to explicitly declare it
pthread_mutex_t DesignationsView::DesignationsLock;
bool DesignationsView::DesignationsLock_Init = false;

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
    
    // If not yet initialized, init the lock
    if(!DesignationsLock_Init)
    {
        pthread_mutex_init(&DesignationsLock, NULL);
        DesignationsLock_Init = true;
    }
}

DesignationsView::~DesignationsView()
{
    // Nothing to release...
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
    DesignationsQueue.Enqueue(ToInsert);
    pthread_mutex_unlock(&DesignationsLock);
}

void DesignationsView::RemoveDesignation(Vector3<int> Point)
{
    // Lock while searching
    pthread_mutex_lock(&DesignationsLock);
    
    // Keep cycling through until we find the one we want to remove
    int DesignationsCount = DesignationsQueue.GetSize();
    for(int i = 0; i < DesignationsCount; i++)
    {
        // Pop off to peek
        Designation* Area = DesignationsQueue.Dequeue();
        
        // If this is the target point, don't add back
        if(Area->Origin != Point)
            DesignationsQueue.Enqueue(Area);
    }
    
    // Done!
    pthread_mutex_unlock(&DesignationsLock);
}

bool DesignationsView::FindDesignation(DesignationType Type, Designation** TargetDesignationOut, Vector3<int>* TargetPosition, Vector3<int>* TargetBlock)
{
    // Lock and copy
    pthread_mutex_lock(&DesignationsLock);
    Queue< Designation* > DesignationQueue = GetDesignations();
    
    // True if a position was found
    bool Found = false;
    
    // Keep cycling through until we find the one we want to remove
    // Note: We attempt to do designations in order of addition
    while(!DesignationQueue.IsEmpty() && !Found)
    {
        // Pop off to peek
        Designation* Area = DesignationQueue.Dequeue();
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
                
                // If it is a half block, we can be both on it and break it
                if(WorldData->IsWithinWorld(*TargetBlock) && !WorldData->GetBlock(*TargetBlock).IsWhole())
                {
                    // We can break the block we are on
                    Found = true;
                    *TargetPosition = *TargetBlock;
                }
                
                // Is this position next to air?
                for(int j = 0; j < AdjacentOffsetsCount && !Found; j++)
                {
                    *TargetPosition = *TargetBlock + AdjacentOffsets[j];
                    if(WorldData->IsWithinWorld(*TargetPosition) && WorldData->GetBlock(*TargetPosition).GetType() == dBlockType_Air)
                        Found = true;
                }
                
                // Push it back only if this isn't a solution
                if(!Found)
                    Area->TaskPositions.Enqueue(*TargetBlock);
            }
        }
    }
    
    // No need for data access
    pthread_mutex_unlock(&DesignationsLock);
    
    // All done
    return Found;
}

Queue< Designation* > DesignationsView::GetDesignations()
{
    // Returns a copy
    return DesignationsQueue;
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
    
    // TESTING: Default to mine
    DwarfJobs TargetJob = DwarfJobs_Miner;
    // Randomly select a job
    /*int Random = rand() % TopJobs.GetSize();
     DwarfJobs TargetJob;
     for(int i = 0; i < Random; i++)
     TopJobs.Dequeue();
     TargetJob = TopJobs.Dequeue();
    */
    
    // Given a job..
    if(TargetJob == DwarfJobs_Miner)
        return GetMiningJob(Dwarf, JobOut);
    
    else if(TargetJob == DwarfJobs_Farmer)
        return GetFarmerJob(Dwarf, JobOut);
    
    else if(TargetJob == DwarfJobs_Crafter)
        return GetCrafterJob(Dwarf, JobOut);
    
    // Undefined
    return false;
}

void DesignationsView::PutBackJob(JobTask* JobOut)
{
    // Simply put back the targetBlock into the designations queue
    pthread_mutex_lock(&DesignationsLock);
    JobOut->TargetDesignation->TaskPositions.Enqueue(JobOut->TargetBlock);
    pthread_mutex_unlock(&DesignationsLock);
}

bool DesignationsView::GetMiningJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    return FindDesignation(DesignationType_Mine, &JobOut->TargetDesignation, &JobOut->TargetPosition, &JobOut->TargetBlock);
}

bool DesignationsView::GetFarmerJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    return FindDesignation(DesignationType_Farm, &JobOut->TargetDesignation, &JobOut->TargetPosition, &JobOut->TargetBlock);
}

bool DesignationsView::GetCrafterJob(DwarfEntity* Dwarf, JobTask* JobOut)
{
    return FindDesignation(DesignationType_Hall, &JobOut->TargetDesignation, &JobOut->TargetPosition, &JobOut->TargetBlock);
}

void DesignationsView::Update(float dT)
{
    // Do nothing...
}

void DesignationsView::Render(int LayerCutoff, bool Draw)
{
    // Lock to copy date
    pthread_mutex_lock(&DesignationsLock);
    Queue< Designation* > DesignationQueue = GetDesignations();
    
    // For each designations
    for(int i = 0; !DesignationQueue.IsEmpty(); i++)
    {
        // Get active area
        Designation* Area = DesignationQueue.Dequeue();
        
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
