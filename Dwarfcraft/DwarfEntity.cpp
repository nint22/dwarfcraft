/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "DwarfEntity.h"

DwarfEntity::DwarfEntity(const char* ConfigName)
: Entity(ConfigName)
{
    // Default values to level 1
    for(int i = 0; i < DwarfJobsCount; i++)
        JobExperiance[i] = 0;
    
    // Default gender and age to male and 32
    IsMale = true;
    Age = 32;
    
    // Rank (new)
    Rank = DwarfRank_Conscript;
    
    // Health (physical and mental) all good
    Happiness = 1.0f;
    Fatigue = 0.0f;
    BreathTime = 10.0f;
    Hunger = 1.0f;
    Thirst = 1.0f;
    
    // Default main and minor jobs to medium priority
    for(int i = 0; i < DwarfJobsCount; i++)
        MainJobs[i] = DwarfJobPriority_Medium;
    for(int i = 0; i < DwarfMinorJobsCount; i++)
        MinorJobs[i] = DwarfJobPriority_Medium;
    
    // No inventory
    Items[0] = dItem();
    Items[1] = dItem();
    
    // Armor set (chest, feet for now)
    Armor[0] = dItem();
    Armor[1] = dItem();
    
    // Initialize the mutex
    ThreadRunningFlag = false;
    pthread_mutex_init(&ThreadMutex, NULL);
    
    // Start with no job
    HasJobFlag = false;
}

DwarfEntity::~DwarfEntity()
{
    // Kill the thread and data mutex
    pthread_kill(ThreadHandle, 0);
    pthread_mutex_destroy(&ThreadMutex);
}

void DwarfEntity::SetArmor(dItem Chest, dItem Legs)
{
    Armor[0] = Chest;
    Armor[1] = Legs;
}

void DwarfEntity::RenderPreview(int x, int y, int width, int height)
{
    // Draw the parent / base object
    Entity::RenderPreview(x, y, width, height);
    
    // Working vars
    float srcx, srcy, srcwidth, srcheight;
    GLuint TextID;
    
    /*** Render Armor ***/
    
    if(Armor[0].Type == dItem_ArmorChest_Steel || Armor[0].Type == dItem_ArmorChest_Leather)
    {
        // Get the current sprite overlay of a chest piece
        GetWearableSprite(Armor[0].Type, &srcx, &srcy, &srcwidth, &srcheight, &TextID);
        
        // Chage cell in the sheet if needed
        srcx += srcwidth * float(GetSpriteCellIndex());
        
        // Render the preview sprite
        Entity::RenderPreview(x, y, width, height, srcx, srcy, srcwidth, srcheight, TextID);
    }
    
    /*** Render Boots ***/
    
    if(Armor[1].Type == dItem_ArmorBoots_Steel || Armor[1].Type == dItem_ArmorBoots_Leather)
    {
        // Get the current sprite overlay of a chest piece
        GetWearableSprite(Armor[1].Type, &srcx, &srcy, &srcwidth, &srcheight, &TextID);
        
        // Chage cell in the sheet if needed
        srcx += srcwidth * float(GetSpriteCellIndex());
        
        // Render the preview sprite
        Entity::RenderPreview(x, y, width, height, srcx, srcy, srcwidth, srcheight, TextID);
    }
}

int DwarfEntity::GetMaxHealth()
{
    // TODO: Should be a function of experiance & armor
    return 0.1f * float(JobExperiance[0] + JobExperiance[1] + JobExperiance[2]);
}

float DwarfEntity::GetHappiness()
{
    return Happiness;
}

float DwarfEntity::GetFatigue()
{
    return Fatigue;
}

float DwarfEntity::GetBreath()
{
    return BreathTime;
}

float DwarfEntity::GetMaxBreath()
{
    // TODO: Some sort of function based on level
    return 10;
}

float DwarfEntity::GetHunger()
{
    return Hunger;
}

float DwarfEntity::GetThirst()
{
    return Thirst;
}

const char* DwarfEntity::GetName()
{
    // TODO: Replace with legitamite name
    return "DWARF HAS NO NAME";
}

int DwarfEntity::GetAge()
{
    return Age;
}

bool DwarfEntity::GetGender()
{
    return IsMale;
}

int DwarfEntity::GetLevel()
{
    // TODO: special function based on experiance
    return 1;
}

int DwarfEntity::GetExp()
{
    // TODO:
    return 1;
}

DwarfRank DwarfEntity::GetRank()
{
    return Rank;
}

DwarfJobPriority* DwarfEntity::GetJobPriority()
{
    return MainJobs;
}

DwarfJobPriority* DwarfEntity::GetMinorJobPriority()
{
    return MinorJobs;
}

void DwarfEntity::Update(float dT)
{
    /*** Status Updates ***/
    
    // Update total time
    static float TotalTime = 0.0f;
    TotalTime += dT;
    
    // TODO... Update thirst, hunger, etc..
    
    /*** Job Error Checking ***/
    
    // Get the error type and reset the error flag (if raised)
    EntityError Error = GetExecutionError();
    
    // If current execution failed, deal with the error
    if(Error != EntityError_None)
    {
        // Give job back (if we have one)
        if(HasJobFlag)
        {
            // State on the console there was a failure
            printf("Entity %d: Unable to complete assigned task!\n", GetEntityID());
            
            GetDesignations()->ResignJob(&Job);
            HasJobFlag = false;
        }
        
        // Otherwise... silently fail
    }
    
    /*** Jobs / Tasks Updates ***/
    
    // If we have no running instructions, but have a job to do, queue up the next steps
    if(!HasInstructions() && HasJob())
    {
        // Build instructions for the given job (i.e. path to it, etc..)
        if(Job.Type == JobType_Mine)
        {
            // Goto adjacent
            EntityInstruction Instr;
            Instr.Operator = EntityOp_MovePath;
            Instr.Data.Path = &JobPath;
            AddInstruction(Instr);
            
            // Break
            Instr.Operator = EntityOP_Break;
            Instr.Data.Pos.x = Job.TargetBlock.x;
            Instr.Data.Pos.y = Job.TargetBlock.y;
            Instr.Data.Pos.z = Job.TargetBlock.z;
            AddInstruction(Instr);
        }
        
        // TODO: Other jobs...
        
    }
    // If has no instructions and has no job and there is no thread running...
    else if(!ThreadRunning() && !HasInstructions() && !HasJob())
    {
        // Lock the running state
        pthread_mutex_lock(&ThreadMutex);
        ThreadRunningFlag = true;
        pthread_mutex_unlock(&ThreadMutex);
        
        // Start execution
        pthread_create(&ThreadHandle, NULL, ComputeTask, (void*)this);
    }
}

void DwarfEntity::Render()
{
    // Render the target tile we are going to (i.e. the end of the path target)
    RenderTargetPath();
    
    // Working vars
    EntityFacing NewGlobalFacing = GetGlobalFacing();
    float x, y, width, height;
    GLuint TextID;
    
    // Always render ontop
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    
    /*** Render Armor ***/
    
    if(Armor[0].Type == dItem_ArmorChest_Steel || Armor[0].Type == dItem_ArmorChest_Leather)
    {
        // Get the current sprite overlay of a chest piece
        GetWearableSprite(Armor[0].Type, &x, &y, &width, &height, &TextID);
        
        // Chage cell in the sheet if needed
        x += width * float(GetSpriteCellIndex());
        
        // Render the sprite (flip as needed)
        Vector2<float> WorldSize = GetWorldSize();
        RenderBillboard(GetPosition(), WorldSize.x, WorldSize.y, x, y, width, height, 0, (NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_BL), TextID);
    }
    
    /*** Render Boots ***/
    
    if(Armor[1].Type == dItem_ArmorBoots_Steel || Armor[1].Type == dItem_ArmorBoots_Leather)
    {
        // Get the current sprite overlay of a chest piece
        GetWearableSprite(Armor[1].Type, &x, &y, &width, &height, &TextID);
        
        // Chage cell in the sheet if needed
        x += width * float(GetSpriteCellIndex());
        
        // Render the sprite (flip as needed)
        Vector2<float> WorldSize = GetWorldSize();
        RenderBillboard(GetPosition(), WorldSize.x, WorldSize.y, x, y, width, height, 0, (NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_BL), TextID);
    }
    
    // Render normal z-fighting render method
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void DwarfEntity::InstructionComplete(EntityInstruction Instr)
{
    // Skip completion check if no job or there is an error
    if(!HasJob())
        return;
    
    // Done with our mining job
    if(Job.Type == JobType_Mine && Instr.Operator == EntityOP_Break)
    {
        GetDesignations()->CompleteJob(&Job);
        HasJobFlag = false;
    }
}

void DwarfEntity::RenderTargetPath()
{
    // Ignore if no job
    Stack< Vector3<int> > Path = GetMovingPath();
    if(Path.IsEmpty())
        return;
    
    // Get the last (target)
    Vector3<int> TilePos;
    while(!Path.IsEmpty())
        TilePos = Path.Pop();
    
    // Push what we will work on
    glPushMatrix();
    
    // Move and scale as needed (shouldn't it scale first then translate?)
    glTranslatef(TilePos.x + 0.5f, TilePos.y + 0.5f, TilePos.z + 0.5f);
    glScalef(0.9f, 0.9f, 0.9f);
    
    // Set color
    float r, g, b;
    g2Label::GetTemplateColor(GetEntityID() % 16, &r, &g, &b);
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    
    // Draw cube
    glLineWidth(2.0f);
    glutWireCube(1.0f);
    
    // Pop this localized coordinate
    glPopMatrix();
}

void* DwarfEntity::ComputeTask(void* data)
{
    /*** Initialize Thread ***/
    
    // Get the entity handle
    DwarfEntity* self = (DwarfEntity*)data;
    
    // Max number of times we attempt to find a task
    static const int MaxAttempts = 3;
    
    // Job object; only gets copied at the end
    JobTask Job;
    
    // Instruction holder, gets pushed as needed
    EntityInstruction Instruction;
    
    /*** Job / Task Generation ***/
    
    // Try a finite amount of times for a job
    bool FoundJob = false;
    for(int Attempt = 0; Attempt < MaxAttempts && !FoundJob; Attempt++)
    {
        // Find a job
        FoundJob = self->GetDesignations()->GetJob(self, &Job);
        Vector3<int> DwarfPosition = self->GetPositionBlock();
        
        // If we didn't find a job, restart
        if(!FoundJob)
            continue;
        // Else, we need to confirm a path ot it (reset flag)
        else
            FoundJob = false;
        
        // Attempt to find a path to it
        for(int i = 0; i < AdjacentOffsetsCount && !FoundJob; i++)
        {
            // Path into the target or directly adjacent to it if that target is air or a half block
            Vector3<int> TargetPosition = Job.TargetBlock + AdjacentOffsets[i];
            dBlock BlockCheck = self->GetWorld()->GetBlock(TargetPosition);
            
            // Is the offset (Job target + adjacent) an open space?
            if(self->GetWorld()->IsWithinWorld(TargetPosition) && (BlockCheck.GetType() == dBlockType_Air || !BlockCheck.IsWhole()))
            {
                // Attempt a path-plan to target
                EntityPath PathCheck(self->GetWorld(), self->GetPositionBlock(), TargetPosition);
                PathCheck.ComputePath();
                
                // Can we reach this path? (Do a busy wait, not a busy stall)
                // (Must like "pthread_yield" but sched_yield is posix) This gives
                // up the thread's allocated proc. time and moves on
                bool IsSolved;
                while(!PathCheck.GetPath(&self->JobPath, &IsSolved))
                    sched_yield();
                
                // This is the first valid path, take it
                // Note: it is up to the dwarf to generate instructions for the job
                if(IsSolved)
                    FoundJob = true;
            }
        }
        
        // Job is not good, resign it
        if(!FoundJob)
            self->GetDesignations()->ResignJob(&Job);
    }
    
    // No job found, go idle
    if(!FoundJob)
    {
        // Switch between either pausing (idle for a few seconds) or pathing
        static int Count = 0;
        if(Count % 2 == 0)
        {
            // Just pause for a few seconds (from 1 to 4)
            Instruction.Operator = EntityOp_Idle;
            Instruction.Data.Idle = 1.0f + 4.0f * (float(rand()) / float(RAND_MAX));
            
            // Push the instruction
            self->AddInstruction(Instruction);
        }
        else
        {
            // Choose a random block in a 3D volume
            static const int Offset = 4;
            Vector3<int> Target(rand() % Offset, rand() % Offset, rand() % Offset);
            
            // Apply the signs
            if(rand() % 2 == 0)
                Target.x *= -1;
            if(rand() % 2 == 0)
                Target.y *= -1;
            if(rand() % 2 == 0)
                Target.z *= -1;
            
            // Add self-pos to generate global target
            Target += self->GetPositionBlock();
            
            // If world bound, add movement instruction
            if(self->GetWorld()->IsWithinWorld(Target.x, Target.y, Target.z))
            {
                // If air, go down
                while(Target.y > 0 && self->GetWorld()->GetBlock(Target.x, Target.y, Target.z).GetType() == dBlockType_Air)
                    Target.y--;
                
                // If non-air, go up
                while(Target.y < self->GetWorld()->GetWorldHeight() && dIsSolid(self->GetWorld()->GetBlock(Target.x, Target.y, Target.z)))
                    Target.y++;
                
                // If above half block, move down
                while(self->GetWorld()->GetBlock(Target + Vector3<int>(0, -1, 0)).IsWhole() == false)
                    Target.y--;
                
                // Add to instruction queue move-to command
                Instruction.Operator = EntityOp_MoveTo;
                Instruction.Data.Pos.x = Target.x;
                Instruction.Data.Pos.y = Target.y;
                Instruction.Data.Pos.z = Target.z;
                
                // Push the instruction
                self->AddInstruction(Instruction);
            }
        }
        
        // Grow no-job execution count
        Count++;
    }
    
    /*** Complete & Post Result ***/
    
    // Lock the running state
    pthread_mutex_lock(&self->ThreadMutex);
    self->ThreadRunningFlag = false;
    self->HasJobFlag = FoundJob;
    self->Job = Job;
    pthread_mutex_unlock(&self->ThreadMutex);
    
    // Nothing to post directly
    return NULL;
}

bool DwarfEntity::ThreadRunning()
{
    bool State = false;
    
    pthread_mutex_lock(&ThreadMutex);
    State = ThreadRunningFlag;
    pthread_mutex_unlock(&ThreadMutex);
    
    return State;
}

bool DwarfEntity::HasJob()
{
    bool State = false;
    
    pthread_mutex_lock(&ThreadMutex);
    State = HasJobFlag;
    pthread_mutex_unlock(&ThreadMutex);
    
    return State;
}
