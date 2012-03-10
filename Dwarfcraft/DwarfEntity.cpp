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
    pthread_mutex_init(&ThreadMutex, NULL);
    ThreadStart = false;
    ThreadDone = false;
}

DwarfEntity::~DwarfEntity()
{
    // Kill the thread
    pthread_kill(ThreadHandle, 0);
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
    
    // Decreass hunger & thirst over time
    Hunger -= 0.003f * dT;
    Thirst -= 0.001f * dT;
    
    // If breathing underwater
    // TODO: is in water block? Decrease breath
    
    // Only do hunger, thurst, and drowning damage every five second
    if(TotalTime >= 5.0f)
    {
        TotalTime -= 5.0f;
        
        // Starving (1 damage)
        if(Hunger <= 0.0f)
            Health -= 1;
        
        // Thirst (1 damage)
        if(Thirst <= 0.0f)
            Health -= 1;
        
        // Drowning (10 damage a second)
        if(BreathTime <= 0.0f)
            Health -= 10;
    }
    
    // Is dead?
    if(Health <= 0)
    {
        // TODO: Flag as dead!
    }
    
    /*** Medical Emergency ***/
    
    if(Hunger <= 0.1f)
    {
        // TODO: Go to nearest food
    }
    else if(Thirst <= 0.1f)
    {
        // TODO: Go to nearest water
    }
    else if(BreathTime <= 0.0f)
    {
        // TODO: Go to nearest air
    }
    
    /*** Ignore Orders if Low Happiness ***/
    
    else if(Happiness <= 0.1f)
    {
        // TODO: Ignore orders
    }
    
    /*** Jobs / Tasks Updates ***/
    
    // If not computing, we need to launch the thread
    else if(!HasInstructions() && !ThreadStarted() && !ThreadComplete())
    {
        // Launch thread
        pthread_create(&ThreadHandle, NULL, ComputeTask, (void*)this);
    }
    
    // Post instructions if complete
    else if(ThreadComplete())
    {
        // Post instructions
        while(!ThreadInstructions.IsEmpty())
            AddInstruction(ThreadInstructions.Dequeue());
        
        // Flag thread as restartable (both not started and not done)
        pthread_mutex_lock(&ThreadMutex);
        ThreadDone = false;
        pthread_mutex_unlock(&ThreadMutex);
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
    
    // Get the corrected position (after the jump)
    Vector3<float> CorrectedPosition = GetPosition() + Vector3<float>(0, GetAnimationJump(), 0);
    
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
        RenderBillboard(CorrectedPosition, WorldSize.x, WorldSize.y, x, y, width, height, 0, (NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_BL), TextID);
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
        RenderBillboard(CorrectedPosition, WorldSize.x, WorldSize.y, x, y, width, height, 0, (NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_BL), TextID);
    }
    
    // Render normal z-fighting render method
    glDisable(GL_POLYGON_OFFSET_FILL);
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
    
    // Go down a tile
    TilePos.y--;
    
    // Only render if we are right below air
    if(GetWorld()->IsWithinWorld(TilePos + Vector3<int>(0, 1, 0)) && GetWorld()->GetBlock(TilePos + Vector3<int>(0, 1, 0)).GetType() == dBlockType_Air)
    {
        // Get texture info
        float srcx, srcy, srcwidth, srcheight; GLuint TextureID;
        dGetDesignationTexture(DesignationType_Mine, &srcx, &srcy, &srcwidth, &srcheight, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        
        glEnable(GL_TEXTURE_2D);
        
        static float phase = 0.0f;
        phase += 0.1f;
        glColor4f(1, 0.8, 0.8, 0.9f + 0.1f * sin(phase));
        
        // Move down 0.5f if half block
        float VerticalOffset = 0.0f;
        if(!GetWorld()->GetBlock(TilePos).IsWhole())
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
    
    glDisable(GL_TEXTURE_2D);
}

void* DwarfEntity::ComputeTask(void* data)
{
    /*** Initialize Thread ***/
    
    // Get the entity handle
    DwarfEntity* self = (DwarfEntity*)data;
    
    // Lock the running state
    pthread_mutex_lock(&self->ThreadMutex);
    self->ThreadStart = true;
    self->ThreadDone = false;
    pthread_mutex_unlock(&self->ThreadMutex);
    
    // Start building an instruction
    EntityInstruction Instruction;
    
    /*** Job / Task Generation ***/
    
    // Copy the job object
    JobTask Job = self->Job;
    
    // Find a job
    bool Success = self->GetDesignations()->GetJob(self, &Job);
    Vector3<int> DwarfPosition = self->GetPositionBlock();
    
    // If success finding a job, see if we can path to it
    if(Success)
    {
        // Do not compute path if trivial
        if(DwarfPosition != Job.TargetPosition)
        {
            // Attempt a path-plan to target
            EntityPath PathCheck(self->GetWorld(), self->GetPositionBlock(), Job.TargetPosition);
            PathCheck.ComputePath();
            
            // Can we reach this path? (Do a busy wait, not a busy stall)
            // (Must like "pthread_yield" but sched_yield is posix) This gives
            // up the thread's allocated proc. time and moves on
            Stack<Vector3<int> > Path;
            while(!PathCheck.GetPath(&Path))
                sched_yield();
            
            // This is the first valid path, take it
            if(Path.GetSize() > 0)
            {
                // Move to it
                Instruction.Operator = EntityOp_MoveTo;
                Instruction.Data.Pos.x = Job.TargetPosition.x;
                Instruction.Data.Pos.y = Job.TargetPosition.y;
                Instruction.Data.Pos.z = Job.TargetPosition.z;
                self->ThreadInstructions.Enqueue(Instruction);
            }
            else
                Success = false;
        }
        
        // Break target block
        if(Success)
        {
            Instruction.Operator = EntityOP_Break;
            Instruction.Data.Pos.x = Job.TargetBlock.x;
            Instruction.Data.Pos.y = Job.TargetBlock.y;
            Instruction.Data.Pos.z = Job.TargetBlock.z;
            self->ThreadInstructions.Enqueue(Instruction);
        }
        // On pathing failure
        else
        {
            // Raise this error into the console (not yet implemented)
            
            // Put this job / task back into queue for another dwarf
            self->GetDesignations()->PutBackJob(&Job);
        }
    }
    
    /*** Idle ***/
    
    // If failed to either find a job or path to it...
    if(!Success)
    {
        // Switch between either pausing (idle for a few seconds) or pathing
        static int Count = 0;
        if(Count % 2 == 0)
        {
            // Just pause for a few seconds (from 1 to 4)
            Instruction.Operator = EntityOp_Idle;
            Instruction.Data.Idle = 1.0f + 4.0f * (float(rand()) / float(RAND_MAX));
            self->ThreadInstructions.Enqueue(Instruction);
            
            // Grow no-job execution count
            Count++;
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
                self->ThreadInstructions.Enqueue(Instruction);
                
                // Grow no-job execution count
                Count++;
            }
        }
    }
    
    /*** Complete & Post Result ***/
    
    // Lock the running state
    pthread_mutex_lock(&self->ThreadMutex);
    self->ThreadStart = false;
    self->ThreadDone = true;
    pthread_mutex_unlock(&self->ThreadMutex);
    
    // Nothing to post directly
    return NULL;
}

bool DwarfEntity::ThreadStarted()
{
    bool State = false;
    
    pthread_mutex_lock(&ThreadMutex);
    State = ThreadStart;
    pthread_mutex_unlock(&ThreadMutex);
    
    return State;
}

bool DwarfEntity::ThreadComplete()
{
    bool State = false;
    
    pthread_mutex_lock(&ThreadMutex);
    State = ThreadDone;
    pthread_mutex_unlock(&ThreadMutex);
    
    return State;
}
