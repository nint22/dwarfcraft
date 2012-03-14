/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "Entity.h"

// Total nubmer of entities instantiated
static int __EntityCount;

// Note the positive preference to render; this is done because
// we want to render entities AFTER the main view's push the camera's
// view-matrix
Entity::Entity(const char* ConfigName)
{
    // Save location
    SetFacingAngle(UtilPI / 2.0f); // Default facing north
    
    // Attempt to read configuration file
    ConfigFile = new g2Config();
    ConfigFile->LoadFile(ConfigName);
    
    // Load the animals texture
    char* str = NULL;
    ConfigFile->GetValue("General", "Texture", &str);
    TextureID = g2LoadImage(str, &TextureWidth, &TextureHeight, NULL, false, false);
    UtilAssert(TextureID != UINT_MAX, "Unable to load art asset for an entity; \"%s\"", str);
    
    // Get target entity size
    Size = 1.0f; // Default
    ConfigFile->GetValue("General", "size", &Size);
    
    // Get the shadow pos / size
    ShadowRadius = 0.25f; // Default
    ConfigFile->GetValue("General", "shadow", &ShadowRadius);
    
    // Default idle_front, a default state
    OldGlobalFacing = GetGlobalFacing();
    WasMoving = false;
    WasBreaking = false;
    if(OldGlobalFacing == EntityFacing_FL || OldGlobalFacing == EntityFacing_FR)
        SetBillboardState("idle_front");
    else
        SetBillboardState("idle_back");
    
    // Load all possible wearables
    for(int i = 0; i < EntityWearablesCount; i++)
    {
        WearablesConfig[i].LoadFile(EntityWearablesConfig[i]);
        WearablesConfig[i].GetValue("General", "Texture", &str);
        
        WearablesTextureID[i] = g2LoadImage(str, &WearblesTextureWidth[i][0], &WearblesTextureWidth[i][1], NULL, false, false);
        UtilAssert(WearablesTextureID[i] != UINT_MAX, "Unable to load art asset for an entity; \"%s\"", str);
    }
    
    // Not currently executing anything
    IsExecuting = false;
    
    // Just idle
    State = OldState = EntityState_Idle;
    
    // Don't render the path by default
    RenderPath = false;
    
    // Path planning starts invalid
    PathPlanner = NULL;
    
    // Not jumping
    IsJumping = false;
    
    // Default to no execution errors
    ExecutionError = EntityError_None;
    
    // Instructions lock
    pthread_mutex_init(&InstructionsLock, NULL);
    
    // Generate an entity ID
    EntityID = __EntityCount++;
    
    /*** Initialize Gameplay Data ***/
    
    // Set health to 10/10
    Health = MaxHealth = 10;
}

Entity::~Entity()
{
    pthread_mutex_destroy(&InstructionsLock);
    delete ConfigFile;
}

g2Config* Entity::GetConfigFile()
{
    return ConfigFile;
}

void Entity::SetPosition(Vector3<int> Pos)
{
    Location = Vector3<float>(Pos.x, Pos.y, Pos.z);
}

void Entity::SetPosition(Vector3<float> Pos)
{
    Location = Pos;
}

float Entity::GetSize()
{
    return Size;
}

Vector2<float> Entity::GetWorldSize()
{
    float width = SpriteSize.x / float(TextureWidth);
    float height = SpriteSize.y / float(TextureHeight);
    return Vector2<float>(Size * (width / height), Size * (height / width));
}

Vector2<int> Entity::GetPixelSize()
{
    return SpriteSize;
}

int Entity::GetEntityID()
{
    return EntityID;
}

int Entity::GetSpriteCellIndex()
{
    return SpriteIndex;
}

Stack< Vector3<int> > Entity::GetMovingPath()
{
    // If valid, then return, else return empty
    if(State == EntityState_Moving)
        return MovingPath;
    else
        return Stack< Vector3<int> >();
}

void Entity::SetPathRender(bool RenderPath)
{
    this->RenderPath = RenderPath;
}

bool Entity::GetPathRender()
{
    return RenderPath;
}

bool Entity::GetBreaking(Vector3<int>* Target, int* Step)
{
    bool IsBreaking = (State == EntityState_Breaking);
    if(IsBreaking)
    {
        *Target = BreakingTarget;
        *Step = BreakingCellIndex;
    }
    
    return IsBreaking;
}

void Entity::SetBreaking(Vector3<int> BreakTarget, float BreakTime)
{
    // Start breaking and save all breaking info
    State = EntityState_Breaking;
    BreakingTarget = BreakTarget;
    BreakingTime = FullBreakingTime = BreakTime;
    BreakingCellIndex = 0;
}

void Entity::AddInstruction(EntityInstruction Instruction)
{
    pthread_mutex_lock(&InstructionsLock);
    InstructionQueue.Enqueue(Instruction);
    pthread_mutex_unlock(&InstructionsLock);
}

void Entity::ClearInstructions()
{
    pthread_mutex_lock(&InstructionsLock);
    while(!InstructionQueue.IsEmpty())
        InstructionQueue.Dequeue();
    pthread_mutex_unlock(&InstructionsLock);
}

bool Entity::GetInstruction(EntityInstruction* Instruction)
{
    pthread_mutex_lock(&InstructionsLock);
    bool success = false;
    if(!InstructionQueue.IsEmpty())
    {
        *Instruction = InstructionQueue.Dequeue();
        success = true;
    }
    pthread_mutex_unlock(&InstructionsLock);
    
    return success;
}

bool Entity::GetActiveInstruction(EntityInstruction* Instruction)
{
    pthread_mutex_lock(&InstructionsLock);
    *Instruction = ActiveInstruction;
    pthread_mutex_unlock(&InstructionsLock);
    return IsExecuting;
}

bool Entity::HasInstructions()
{
    pthread_mutex_lock(&InstructionsLock);
    bool success = false;
    if(InstructionQueue.GetSize() > 0 || IsExecuting)
        success = true;
    pthread_mutex_unlock(&InstructionsLock);
    
    return success;
}

void Entity::RaiseExecutionError(EntityError Error)
{
    // Save the error
    ExecutionError = Error;
    
    // Release all instructions
    IsExecuting = false;
    ClearInstructions();
    
    // Push a stall instruction
    EntityInstruction Instr;
    Instr.Operator = EntityOp_Idle;
    Instr.Data.Idle = 1.0f;
    AddInstruction(Instr);
}

EntityError Entity::GetExecutionError()
{
    EntityError Error = ExecutionError;
    ExecutionError = EntityError_None;
    return Error;
}

void Entity::SetFacingAngle(float Theta)
{
    this->FacingTheta = Theta;
}

float Entity::GetFacingAngle()
{
    return FacingTheta;
}

float Entity::GetCameraAngle()
{
    return CameraTheta;
}

void Entity::RenderPreview(int x, int y, int width, int height)
{
    // Generate texture position for the regular dwarf texture
    float srcwidth = SpriteSize.x / float(TextureWidth);
    float srcheight = SpriteSize.y / float(TextureHeight);
    float srcx = SpritePos.x / float(TextureWidth) + srcwidth * float(SpriteIndex);
    float srcy = SpritePos.y / float(TextureHeight);
    
    RenderPreview(x, y, width, height, srcx, srcy, srcwidth, srcheight);
}

void Entity::RenderPreview(int x, int y, int width, int height, float srcx, float srcy, float srcwidth, float srcheight, GLuint TextureID)
{
    // Default texture ID
    if(TextureID == INT_MAX)
        TextureID = this->TextureID;
    
    float TextureCoords[4][2] =
    {
        {srcx, srcy},
        {srcx, srcy + srcheight},
        {srcx + srcwidth, srcy + srcheight},
        {srcx + srcwidth, srcy},
    };
    
    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    
    // Render a 0.5 0.5 bilboard at the target position
    glPushMatrix();
    {
        // Initialize color, rotation, and location
        glColor3f(1, 1, 1);
        glTranslatef(x, y, 0);
        
        // Front facing sprite
        glBegin(GL_QUADS);
        {
            glTexCoord2f(TextureCoords[0][0], TextureCoords[0][1]);
            glVertex2f(0, 0);
            
            glTexCoord2f(TextureCoords[1][0], TextureCoords[1][1]);
            glVertex2f(0, height);
            
            glTexCoord2f(TextureCoords[2][0], TextureCoords[2][1]);
            glVertex2f(width, height);
            
            glTexCoord2f(TextureCoords[3][0], TextureCoords[3][1]);
            glVertex2f(width, 0);
        }
        glEnd();
    }
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

int Entity::GetHealth()
{
    return Health;
}

int Entity::GetMaxHealth()
{
    return MaxHealth;
}

const char* Entity::GetName()
{
    char* Name = NULL;
    ConfigFile->GetValue("General", "Name", &Name);
    return Name;
}

void Entity::Update(float dT)
{
    // To be overloaded by the deriving class
}

void Entity::Render()
{
    // To be overloaded by the deriving class
}

void Entity::InstructionComplete(EntityInstruction Instr)
{
    // To be overloaded by the deriving class
}

void Entity::__Update(float dT)
{
    /*** Get and Execute Instruction ***/

    // Get the derived entity to update
    Update(dT);
    
    // Execute instruction update as needed
    Execute(dT);
    
    // Update for the physics
    UpdatePhys(dT);
    
    /*** Sprite Animation ***/
    
    // Total time
    SpriteTimer += dT;
    
    // Change frame as needed
    if(SpriteTimer > SpriteDelay)
    {
        // Change frame
        SpriteIndex = (SpriteIndex + 1) % SpriteFrames;
        
        // Reset timer
        SpriteTimer = 0.0f;
    }
}

void Entity::__Render(float CameraAngle)
{
    // Save this new camera angle for internal use
    CameraTheta = CameraAngle;
    
    /*** Main Sprite ***/
    
    // Check for any facing changes, or state changes
    EntityFacing NewGlobalFacing = GetGlobalFacing();
    if(OldGlobalFacing != NewGlobalFacing || OldState != State)
    {
        // Change sprite sheet if mining
        if(State == EntityState_Breaking)
        {
            if(NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_FR)
                SetBillboardState("mine_front");
            else
                SetBillboardState("mine_back");
        }
        
        // Change sprite sheet to idle
        else if(State == EntityState_Idle)
        {
            if(NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_FR)
                SetBillboardState("idle_front");
            else
                SetBillboardState("idle_back");
        }
        
        // Is moving...
        else if(State == EntityState_Moving)
        {
            if(NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_FR)
                SetBillboardState("move_front");
            else
                SetBillboardState("move_back");
        }
        
        // Save new facing
        OldGlobalFacing = NewGlobalFacing;
        OldState = State;
    }
    
    // Generate texture position
    float width = SpriteSize.x / float(TextureWidth);
    float height = SpriteSize.y / float(TextureHeight);
    float x = SpritePos.x / float(TextureWidth) + width * float(SpriteIndex);
    float y = SpritePos.y / float(TextureHeight);
    
    // Render the sprite (flip as needed)
    Vector2<float> WorldSize = GetWorldSize();
    RenderBillboard(GetPosition(), WorldSize.x, WorldSize.y, x, y, width, height, 0.0f, (NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_BL));
    
    /*** Shadows ***/
    
    // We only care about the first solid block under this dwarf (including which block it is in)
    dBlock SolidBlock(dBlockType_Air);
    float ground;
    
    for(int y = Location.y; y >= 0; y--)
    {
        SolidBlock = GetWorld()->GetBlock(Location.x, y, Location.z);
        if(SolidBlock.GetType() != dBlockType_Air)
        {
            // If solid block, the ground is 1 unit above
            if(SolidBlock.IsWhole())
                ground = y + 1;
            else
                ground = y + 0.5f;
            break;
        }
    }
    
    // Render shadow (offset a little up)
    RenderShadow(Vector3<float>(GetPosition().x, ground + 0.01f, GetPosition().z), ShadowRadius);
    
    /*** Custom Rendering ***/
    
    // Render any special entity-specific properties
    Render();
}

void Entity::UpdatePhys(float dT)
{
    // How fast we fall in a cycle
    static const float fall_vel = 0.05f;
    
    /*** In a block, pop up ***/
    
    // If in a solid whole block
    dBlock CurrentBlock = GetWorld()->GetBlock(Location);
    if(CurrentBlock.IsWhole() && CurrentBlock.GetType() != dBlockType_Air)
    {
        // Pop up and set to that location
        CurrentBlock = GetWorld()->GetBlock(Location + Vector3<float>(0, 1, 0));
        if(CurrentBlock.IsWhole())
            Location.y = (int)Location.y + 1.0f;
        else
            Location.y = (int)Location.y + 0.5f;
    }
    // Or within but below a half block
    else if(!CurrentBlock.IsWhole() && (Location.y - (int)Location.y) < 0.5f)
       Location.y = (int)Location.y + 0.5f;
    
    /*** Falling while in the air ***/
    
    // We only care about the first solid block under this dwarf (including which block it is in)
    dBlock SolidBlock(dBlockType_Air);
    float ground;
    
    for(int y = Location.y; y >= 0; y--)
    {
        SolidBlock = GetWorld()->GetBlock(Location.x, y, Location.z);
        if(SolidBlock.GetType() != dBlockType_Air)
        {
            // If solid block, the ground is 1 unit above
            if(SolidBlock.IsWhole())
                ground = y + 1;
            else
                ground = y + 0.5f;
            break;
        }
    }
    
    // Fail out: dwarf is somehow under the world?
    UtilAssert(SolidBlock.GetType() != dBlockType_Air, "Dwarf is falling out of the world");
    
    // If the dwarf's loction is above the ground plane, slowely bring it down, but never below the plane itself
    if(Location.y > ground + fall_vel)
        Location.y -= fall_vel;
}

void Entity::Execute(float dT)
{
    // Update timer
    if(IsExecuting)
        ExecutionTime += dT;
    else
        ExecutionTime = 0.0f;
    
    // Get a new instruction if needed
    if(!IsExecuting)
    {
        // Attempt to get instruction
        if(GetInstruction(&ActiveInstruction))
            IsExecuting = true;
        else
            return;
    }
    
    /*** Instruction Implementation List ***/
    
    // If stalling
    if(ActiveInstruction.Operator == EntityOp_Idle)
    {
        // Done when over required time
        if(ExecutionTime > ActiveInstruction.Data.Idle)
            IsExecuting = false;
    }
    
    // If generating path, but not already moving
    else if(ActiveInstruction.Operator == EntityOp_MoveTo)
    {
        // Path s actually valid
        bool IsValidPath;
        
        // Actively moving
        if(State == EntityState_Moving)
        {
            // Interally, it will turn off "IsExecuting"
            ExecuteMove(dT);
        }
        // Not yet started computing
        else if(PathPlanner == NULL)
        {
            // Allocate and launch thread
            PathPlanner = new EntityPath(MainWorld, GetPositionBlock(), Vector3<int>(ActiveInstruction.Data.Pos.x, ActiveInstruction.Data.Pos.y, ActiveInstruction.Data.Pos.z));
            PathPlanner->ComputePath();
            
            // Empty current paths
            while(!MovingPath.IsEmpty())
                MovingPath.Pop();
        }
        // Else, attempt to get the path
        else if(PathPlanner->GetPath(&MovingPath, &IsValidPath))
        {
            // If empty, then it is an invalid path
            if(MovingPath.GetSize() > 0 && IsValidPath)
            {
                // Stop instruction execution and start movement
                State = EntityState_Moving;
                
                // Pop off the starting location, and ignore path movement if none left
                Vector3<int> StartingPos = MovingPath.Pop();
                if(!LocalizePosition(StartingPos, &AnimationSource))
                    RaiseExecutionError(EntityError_Blocked);
                
                // Trivial path: at self-location
                if(MovingPath.IsEmpty())
                {
                    State = EntityState_Idle;
                    IsExecuting = false;
                }
            }
            // Error out
            else
                RaiseExecutionError(EntityError_BadPath);
            
            // Regardless of success, release the path planner object
            delete PathPlanner;
            PathPlanner = NULL;
        }
    }
    
    // Path already generated, just execute movement
    else if(ActiveInstruction.Operator == EntityOp_MovePath)
    {
        // Actively moving
        if(State == EntityState_Moving)
        {
            // Interally, it will turn off "IsExecuting"
            ExecuteMove(dT);
        }
        // Not yet started computing
        else
        {
            // Copy given path into MovingPath
            MovingPath = *ActiveInstruction.Data.Path;
            
            // Stop instruction execution and start movement
            State = EntityState_Moving;
            
            // Pop off the starting location, and ignore path movement if none left
            Vector3<int> StartingPos = MovingPath.Pop();
            if(!LocalizePosition(StartingPos, &AnimationSource))
                RaiseExecutionError(EntityError_Blocked);
            
            // Trivial path: at self-location
            if(MovingPath.IsEmpty())
            {
                State = EntityState_Idle;
                IsExecuting = false;
            }
        }
    }
    
    // If we are breaking a block
    else if(ActiveInstruction.Operator == EntityOP_Break)
    {
        // Actively breaking
        if(State == EntityState_Breaking)
        {
            // Internally it will eventuall stop execution once complete
            ExecuteBreak(dT);
        }
        else
        {
            // We can only break blocks directly adjacent or within us
            static const int OffsetCount = 7;
            static const Vector3<int> Offsets[OffsetCount] = {
                Vector3<int>(0, 0, 0),
                Vector3<int>(1, 0, 0),
                Vector3<int>(-1, 0, 0),
                Vector3<int>(0, 1, 0),
                Vector3<int>(0, -1, 0),
                Vector3<int>(0, 0, 1),
                Vector3<int>(0, 0, -1),
            };
            
            // Get the block coord we want to destroy
            Vector3<int> TargetBlock(ActiveInstruction.Data.Pos.x, ActiveInstruction.Data.Pos.y, ActiveInstruction.Data.Pos.z);
            
            // Block we want to break has to be directly adjacent
            bool IsValid = false;
            for(int i = 0; i < OffsetCount; i++)
            {
                if(GetPositionBlock() == (TargetBlock + Offsets[i]))
                {
                    IsValid = true;
                    break;
                }
            }
            
            // If adjacent.. change block to debugging
            if(IsValid)
            {
                // TODO: need to pass correct content here
                SetBreaking(TargetBlock, dGetBreakTime(0, dBlock(dBlockType_Dirt)));
            }
            // Give up by posting an error
            else
                RaiseExecutionError(EntityError_NotReachable);
        }
    }
    
    // If changing face
    else if(ActiveInstruction.Operator == EntityOp_Face)
    {
        // Change face instantly
        SetFacingAngle(ActiveInstruction.Data.Face);
        IsExecuting = false;
    }
    
    // If placing item
    else if(ActiveInstruction.Operator == EntityOp_Pickup)
    {
        // What item do we want to take
        // TODO
    }
    
    // If droping off item
    else if(ActiveInstruction.Operator == EntityOp_Dropoff)
    {
        // Which item of our two we want to drop off?
        // TODO
    }
    
    // Else, undefined op; internal error
    else
        UtilAssert(false, "Unknown op \"%d\" from entity \"%d\" attempted to execute", (int)ActiveInstruction.Operator, GetEntityID());
    
    // If done executing, that means we are done with this instruction
    if(IsExecuting == false && ExecutionError == EntityError_None)
        InstructionComplete(ActiveInstruction);
}

void Entity::ExecuteMove(float dT)
{
    // Are we done traveling to our target?
    if(MovingPath.IsEmpty())
    {
        // Done moving; make sure we are at the precise right location
        State = EntityState_Idle;
        IsExecuting = false;
    }
    // Commit to animation
    else
    {
        /*** Walking Movement / Update ***/
        
        // Current target
        Vector3<float> AnimationTarget;
        if(!LocalizePosition(MovingPath.Peek(), &AnimationTarget))
            RaiseExecutionError(EntityError_Blocked);
        
        // Find the vector / direction
        Vector3<float> Direction = AnimationTarget - Location;
        Direction.Normalize();
        
        // Set the facing angle
        float Angle = atan2(Direction.x, Direction.z) + UtilPI / 2.0f;
        SetFacingAngle(Angle);
        
        // Compute current location
        Location += Direction * 0.025f; // TODO: ENTITY SPEED HERE
        
        /*** Jumping Animation ***/
        
        // Note: These distances are on the xz-plane, and does not include the y-component
        
        // What is the distance to the target?
        float CurrentDistance = (Vector2<float>(Location.x, Location.z) - Vector2<float>(AnimationSource.x, AnimationSource.z)).GetLength();
        
        // What is the distance from source to destination?
        float TotalDistance = (Vector2<float>(AnimationTarget.x, AnimationTarget.z) - Vector2<float>(AnimationSource.x, AnimationSource.z)).GetLength();
        
        // Current block we are on
        dBlock CurrentBlock = GetWorld()->GetBlock(Vector3ftoi(AnimationSource));
        
        // Next block we will go to
        dBlock NextBlock = GetWorld()->GetBlock(Vector3ftoi(AnimationTarget));
        
        // If the entity is near the middle of a transition
        static const float AnimationJump_Threshold = 0.35f; // From the center
        float TransitionDelta = fabs((TotalDistance / 2.0f) - CurrentDistance);// / TotalDistance;
        if(TransitionDelta <= AnimationJump_Threshold && (CurrentBlock.IsWhole() != NextBlock.IsWhole() || int(AnimationSource.y) != int(AnimationTarget.y)))
        {
            IsJumping = true;
            
            // From start to end, not from middle
            float Normalized = (CurrentDistance - (TotalDistance / 2.0f - AnimationJump_Threshold)) / (AnimationJump_Threshold * 2);
            Location.y += 0.15f * sin(UtilPI / 2.0f + UtilPI * Normalized);
        }
        else
            IsJumping = false;
        
        /*** Reached Animation Sink ***/
        
        // If our distance is small, then pop the target and start the timer again
        // Make sure we don't overshoot either
        if(CurrentDistance >= TotalDistance)
        {
            // Save the new source position
            Vector3<int> StartingPos = MovingPath.Pop();
            if(!LocalizePosition(StartingPos, &AnimationSource))
                RaiseExecutionError(EntityError_Blocked);
        }
    }
}

void Entity::ExecuteBreak(float dT)
{
    // Countdown the breaking timer
    BreakingTime -= dT;
    
    // If we broke it, stop executing the break instruction
    if(BreakingTime <= 0.0f)
    {
        // Stop breaking state
        State = EntityState_Idle;
        IsExecuting = false;
        
        // Place the coal onto the ground now; then replace block to air
        GetItems()->AddItem(dItem(dItem_Coal), BreakingTarget);
        GetWorld()->SetBlock(BreakingTarget.x, BreakingTarget.y, BreakingTarget.z, dBlockType_Air);
    }
    // Else, post the progress
    else
    {
        // Cell Index = (normalized progress from 0 to 1) * 7 [Cell count]
        BreakingCellIndex = ((FullBreakingTime - BreakingTime) / FullBreakingTime) * 7;
    }
}

WorldContainer* Entity::GetWorld()
{
    return MainWorld;
}

DesignationsView* Entity::GetDesignations()
{
    return Designations;
}

ItemsView* Entity::GetItems()
{
    return Items;
}

Entities* Entity::GetEntities()
{
    return WorldEntities;
}

Vector3<float> Entity::GetPosition()
{
    return Location;
}

Vector3<int> Entity::GetPositionBlock()
{
    return Vector3<int>(Location.x, Location.y, Location.z);
}

void Entity::SetBillboardState(const char* State)
{
    // Reset timers and animation index
    SpriteTimer = 0.0f;
    SpriteIndex = 0;
    
    // Get position
    GetConfigFile()->GetValue(State, "position", &SpritePos.x, &SpritePos.y);
    
    // Get size
    GetConfigFile()->GetValue(State, "size", &SpriteSize.x, &SpriteSize.y);
    
    // Get frame count
    GetConfigFile()->GetValue(State, "frames", &SpriteFrames);
    
    // Get delay
    GetConfigFile()->GetValue(State, "delay", &SpriteDelay);
}

void Entity::RenderBillboard(Vector3<float> pos, float outwidth, float outheight, float srcx, float srcy, float srcwidth, float srcheight, float doffset, bool flip, GLuint TextureID)
{
    // Default texture ID
    if(TextureID == INT_MAX)
        TextureID = this->TextureID;
    
    float TextureCoords[4][2] =
    {
        {srcx + srcwidth, srcy + srcheight},
        {srcx, srcy + srcheight},
        {srcx, srcy},
        {srcx + srcwidth, srcy},
    };
    
    // Flip on the y axis the texture if facing left
    if(flip)
    {
        // Bottom x-coordinates
        float temp = TextureCoords[0][0];
        TextureCoords[0][0] = TextureCoords[1][0];
        TextureCoords[1][0] = temp;
        
        // Top x-coordinates
        temp = TextureCoords[2][0];
        TextureCoords[2][0] = TextureCoords[3][0];
        TextureCoords[3][0] = temp;
    }
    
    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    
    // Render a 0.5 0.5 bilboard at the target position
    glPushMatrix();
    {
        // Initialize color, rotation, and location
        glColor3f(1, 1, 1);
        
        glTranslatef(pos.x, pos.y, pos.z);
        glRotatef(UtilRadToDeg * (GetCameraAngle()  - UtilPI / 2.0f), 0, 1, 0);
        
        // Front facing sprite
        glBegin(GL_QUADS);
        {
            glTexCoord2f(TextureCoords[0][0], TextureCoords[0][1]);
            glVertex3f(-outwidth / 2.0f, 0, 0);
            
            glTexCoord2f(TextureCoords[1][0], TextureCoords[1][1]);
            glVertex3f(outwidth / 2.0f, 0, 0);
            
            glTexCoord2f(TextureCoords[2][0], TextureCoords[2][1]);
            glVertex3f(outwidth / 2.0f, outheight, 0);
            
            glTexCoord2f(TextureCoords[3][0], TextureCoords[3][1]);
            glVertex3f(-outwidth / 2.0f, outheight, 0);
        }
        glEnd();
    }
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void Entity::RenderShadow(Vector3<float> pos, float radius)
{
    // Total segments
    static const int Segments = 12;
    
    // Start a circle at the face
    // About the center, above ground
    glPushMatrix();
    glTranslatef(pos.x, pos.y + 0.01f, pos.z);
    glColor4f(0, 0, 0, 0.5f);
    
    glBegin(GL_TRIANGLE_FAN);
    
    glVertex3f(0, 0, 0); // Center point
    for(int i = 0; i <= Segments; i++)
    {
        float n = float(i) * (UtilPI / (Segments * 0.5f));
        glVertex3f(-cos(n) * radius, 0, sin(n) * radius);
    }
    
    glEnd();
    
    // Finished
    glPopMatrix();
}

EntityFacing Entity::GetGlobalFacing()
{
    // Figure out our sprite's facing direction
    // Defaulted to something, even though we explicitly check all cases
    EntityFacing Facing = EntityFacing_FR;
    
    // Compute the real difference between the two angles
    float AngleDiff = GetCameraAngle() - GetFacingAngle();
    while (AngleDiff < -UtilPI)
        AngleDiff += 2.0f * UtilPI;
    while (AngleDiff > UtilPI)
        AngleDiff -= 2.0f * UtilPI;
    
    // What is the front limit (the angle at which we show the face)
    const float FrontLimit = UtilPI / 2.0f;
    
    // If camera is within 45 angles of either one.. we are facing it
    if(AngleDiff > -FrontLimit && AngleDiff <= 0.0f)
        Facing = EntityFacing_FR;
    else if(AngleDiff < FrontLimit && AngleDiff > 0.0f)
        Facing = EntityFacing_FL;
    else if(AngleDiff > 0.0f)
        Facing = EntityFacing_BL;
    else if(AngleDiff < 0.0f)
        Facing = EntityFacing_BR;
    
    return Facing;
}

bool Entity::GetWearableSprite(dItemType ItemType, float* x, float* y, float* width, float* height, GLuint* TextureID)
{
    // Slow, but currently lowest memory overhead
    for(int i = 0; i < EntityWearablesCount; i++)
    {
        // Found
        if(EntityWearablesType[i] == ItemType)
        {
            // Current facing state
            EntityFacing NewGlobalFacing = GetGlobalFacing();
            
            // Place appropriate animations
            if(State == EntityState_Breaking)
            {
                if(NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_FR)
                {
                    WearablesConfig[i].GetValue("mine_front", "position", x, y);
                    WearablesConfig[i].GetValue("mine_front", "size", width, height);
                }
                else
                {
                    WearablesConfig[i].GetValue("mine_back", "position", x, y);
                    WearablesConfig[i].GetValue("mine_back", "size", width, height);
                }
            }
            else if(State == EntityState_Idle)
            {
                if(NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_FR)
                {
                    WearablesConfig[i].GetValue("idle_front", "position", x, y);
                    WearablesConfig[i].GetValue("idle_front", "size", width, height);
                }
                else
                {
                    WearablesConfig[i].GetValue("idle_back", "position", x, y);
                    WearablesConfig[i].GetValue("idle_back", "size", width, height);
                }
            }
            else if(State == EntityState_Moving)
            {
                if(NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_FR)
                {
                    WearablesConfig[i].GetValue("move_front", "position", x, y);
                    WearablesConfig[i].GetValue("move_front", "size", width, height);
                }
                else
                {
                    WearablesConfig[i].GetValue("move_back", "position", x, y);
                    WearablesConfig[i].GetValue("move_back", "size", width, height);
                }
            }
            
            // Convert to sprite coordinates
            *x /= float(WearblesTextureWidth[i][0]);
            *y /= float(WearblesTextureWidth[i][1]);
            *width /= float(WearblesTextureWidth[i][0]);
            *height /= float(WearblesTextureWidth[i][1]);
            
            // Post texture ID
            *TextureID = WearablesTextureID[i];
            
            // All done!
            return true;
        }
    }
    
    // Never found...
    return false;
}

bool Entity::LocalizePosition(Vector3<int> Pos, Vector3<float>* PosOut)
{
    // Target block we are moving into
    dBlock InBlock = GetWorld()->GetBlock(Pos);
    dBlock BottomBlock = GetWorld()->GetBlock(Pos + Vector3<int>(0, -1, 0));
    
    // Go into block if halfblock
    if(!InBlock.IsWhole() && InBlock.GetType() != dBlockType_Air)
    {
        *PosOut = Vector3<float>(Pos.x + 0.5f, Pos.y + 0.5f, Pos.z + 0.5f);
        return true;
    }
    // Go ontop of block
    else if(InBlock.IsWhole() && InBlock.GetType() == dBlockType_Air &&
            BottomBlock.IsWhole() && BottomBlock.GetType() != dBlockType_Air)
    {
        *PosOut = Vector3<float>(Pos.x + 0.5f, Pos.y, Pos.z + 0.5f);
        return true;
    }
    // Not valid
    else
        return false;
}
