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
    
    // No jumping offset yet
    AnimationJump = 0.0f;
    
    // Generate an entity ID
    EntityID = __EntityCount++;
    
    /*** Initialize Gameplay Data ***/
    
    // Set health to 10/10
    Health = MaxHealth = 10;
}

Entity::~Entity()
{
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

float Entity::GetAnimationJump()
{
    return AnimationJump;
}

void Entity::AddInstruction(EntityInstruction Instruction)
{
    InstructionQueue.Enqueue(Instruction);
}

void Entity::ClearInstructions()
{
    while(!InstructionQueue.IsEmpty())
        InstructionQueue.Dequeue();
}

bool Entity::GetInstruction(EntityInstruction* Instruction)
{
    if(InstructionQueue.IsEmpty())
        return false;
    else
    {
        *Instruction = InstructionQueue.Dequeue();
        return true;
    }
}

bool Entity::GetActiveInstruction(EntityInstruction* Instruction)
{
    *Instruction = ActiveInstruction;
    return IsExecuting;
}

bool Entity::HasInstructions()
{
    if(InstructionQueue.GetSize() <= 0 && !IsExecuting)
        return false;
    else
        return true;
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

void Entity::__Update(float dT)
{
    /*** Get and Execute Instruction ***/

    // Get the derived entity to update
    Update(dT);
    
    // Execute instruction update as needed
    Execute(dT);
    
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
    RenderBillboard(GetPosition() + Vector3<float>(0, AnimationJump, 0), WorldSize.x, WorldSize.y, x, y, width, height, 0.0f, (NewGlobalFacing == EntityFacing_FL || NewGlobalFacing == EntityFacing_BL));
    
    // Render shadow
    RenderShadow(GetPosition(), ShadowRadius);
    
    /*** Custom Rendering ***/
    
    // Render any special entity-specific properties
    Render();
    
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
        // Actively moving
        if(State == EntityState_Moving)
        {
            // Interally, it will turn off "IsExecuting"
            ExecuteMove(dT);
        }
        // Not yet started computing
        else if(PathPlanner == NULL)
        {
            // Save target
            MovingTarget = Vector3<int>(ActiveInstruction.Data.Pos.x, ActiveInstruction.Data.Pos.y, ActiveInstruction.Data.Pos.z);
            
            // Allocate and launch thread
            PathPlanner = new EntityPath(MainWorld, GetPositionBlock(), MovingTarget);
            PathPlanner->ComputePath();
            
            // Empty current paths
            while(!MovingPath.IsEmpty())
                MovingPath.Pop();
        }
        // Else, attempt to get the path
        else if(PathPlanner->GetPath(&MovingPath))
        {
            // If empty, then it is an invalid path
            if(MovingPath.GetSize() > 0)
            {
                // Stop instruction execution and start movement
                State = EntityState_Moving;
                
                // Pop off the starting location, and ignore path movement if none left
                Vector3<int> StartingPos = MovingPath.Pop();
                AnimationSource = Vector3<float>(StartingPos.x, StartingPos.y, StartingPos.z);
                
                // Trivial path: at self-location
                if(MovingPath.IsEmpty())
                {
                    State = EntityState_Idle;
                    IsExecuting = false;
                }
            }
            // Error out
            else
            {
                g2ChatController::printf(this, "\\04Warning: \\0Dwarf \"%s\" cannot get to target!", GetName());
                g2ChatController::printf(GetPositionBlock(), "    From position: %d %d %d", GetPositionBlock().x, GetPositionBlock().y, GetPositionBlock().z);
                g2ChatController::printf(MovingTarget, "    Target position: %d %d %d", MovingTarget.x, MovingTarget.y, MovingTarget.z);
                IsExecuting = false;
            }
            
            // Regardless of success, release the path planner object
            delete PathPlanner;
            PathPlanner = NULL;
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
            // List of all directly adjacent blocks to check
            static const Vector3<int> Offsets[6] = {
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
            for(int i = 0; i < 6; i++)
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
            // Give up
            else
            {
                IsExecuting = false;
                printf("WARNING: Unable to reach from source <%d, %d, %d> to target <%d, %d, %d> for EntityOP_Break!\n", GetPositionBlock().x, GetPositionBlock().y, GetPositionBlock().z, TargetBlock.x, TargetBlock.y, TargetBlock.z);
            }
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
        UtilAssert(false, "Unknown op \"%d\" entity \"%d\" attempted to execute", (int)ActiveInstruction.Operator, GetEntityID());
}

void Entity::ExecuteMove(float dT)
{
    // Are we done traveling to our target?
    if(MovingPath.IsEmpty())
    {
        // Done moving; make sure we are at the precise right location
        State = EntityState_Idle;
        IsExecuting = false;
        Location = Vector3<float>(MovingTarget.x, MovingTarget.y, MovingTarget.z);
        
        // If half block, move up..
        if(!GetWorld()->GetBlock(MovingTarget).IsWhole())
            Location.y += 0.5f;
        
        // Make sure to reset the jumping animation
        AnimationJump = 0.0f;
    }
    // Commit to animation
    else
    {
        /*** Walking Movement / Update ***/
        
        // Current target
        Vector3<float> AnimationTarget = Vector3<float>(MovingPath.Peek().x, MovingPath.Peek().y, MovingPath.Peek().z);
        
        // If half block, move up..
        if(!GetWorld()->GetBlock(MovingPath.Peek()).IsWhole())
            AnimationTarget.y += 0.5f;
        
        // Find the vector / direction
        Vector3<float> Direction = AnimationTarget - Location;
        Direction.Normalize();
        
        // Set the facing angle
        float Angle = atan2(Direction.x, Direction.z) + UtilPI / 2.0f;
        SetFacingAngle(Angle);
        
        // Compute current location
        Location += Direction * 0.025f; // TODO: ENTITY SPEED HERE
        
        // What is the distance to the target?
        float CurrentDistance = (Location - AnimationSource).GetLength();
        
        // What is the distance from source to destination?
        float TotalDistance = (AnimationTarget - AnimationSource).GetLength();
        
        /*** Jumping Animation ***/
        
        // Current block we are on
        dBlock CurrentBlock = GetWorld()->GetBlock(Vector3ftoi(AnimationSource));
        
        // Next block we will go to
        dBlock NextBlock = GetWorld()->GetBlock(Vector3ftoi(AnimationTarget));
        
        // If the entity is near the middle of a transition
        static const float AnimationJump_Threshold = 0.2f;
        float TransitionDelta = fabs((TotalDistance / 2.0f) - CurrentDistance) / TotalDistance;
        if(TransitionDelta <= AnimationJump_Threshold && (CurrentBlock.IsWhole() != NextBlock.IsWhole() || int(AnimationSource.y) != int(AnimationTarget.y)))
            AnimationJump = 1.5f * (AnimationJump_Threshold - TransitionDelta);
        else
            AnimationJump = 0.0f;
        
        /*** Reached Animation Sink ***/
        
        // If our distance is small, then pop the target and start the timer again
        // Make sure we don't overshoot either
        if(CurrentDistance >= TotalDistance)
        {
            // Save the new source position
            Vector3<int> StartingPos = MovingPath.Pop();
            AnimationSource = Vector3<float>(StartingPos.x, StartingPos.y, StartingPos.z);
            
            // Update animation source so we know to level off corectly
            if(!GetWorld()->GetBlock(StartingPos).IsWhole())
                AnimationSource.y += 0.5f;
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
        
        // TODO: Map the block to the item
        // Place the coal onto the ground now; then replace block to air
        GetItems()->AddItem(dItem(dItem_Coal), BreakingTarget);
        GetWorld()->SetBlock(BreakingTarget.x, BreakingTarget.y, BreakingTarget.z, dBlockType_Air);
        
        // Layer above, including, and below needs updates
        //for(int i = BreakingTarget.y - 1; i <= BreakingTarget.y + 1; i++)
        //    GetWorld()->SetUpdateState(i, true);
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
    
    // Compute the view-ray offsets (so we can order textures)
    Vector2<float> Offset(cos(-GetCameraAngle()), sin(-GetCameraAngle()));
    if(doffset >= 0.001f || doffset <= -0.001f)
        Offset *= doffset;
    else
        Offset = Vector2<float>(0.0f, 0.0f);
    
    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    
    // Render a 0.5 0.5 bilboard at the target position
    glPushMatrix();
    {
        // Initialize color, rotation, and location
        glColor3f(1, 1, 1);
        
        glTranslatef(pos.x + 0.5f + Offset.x, pos.y, pos.z + 0.5f + Offset.y);
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
    glPushMatrix();
    glTranslatef(pos.x + 0.5f, pos.y + 0.01f, pos.z + 0.5f);
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
