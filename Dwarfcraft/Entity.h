/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Entity.h/cpp
 Desc: Any on-screen "intelligent" entity, such as an enemy,
 friendly, player, or neutral entity. Renders as a 2D bilboard.
 Managed by the entities class.
 
 Certain entity properties are defined in the associated
 configuration file which is an ini-like *.cfg file. The
 following properties must be defined per entity. Please note
 that the AI is implemented at the code leve; though there
 might be a "cow.cfg" to define cow properties, the implementation
 is associated with the "AnimalsEntity class".
 
 [General]
 name: <short name>
 description: <description>
 texture: <texture file location>
 size: <float; world size of mob>
 shadow: float
 
 [<animation name; one word>]
 position: <integer tuple of where the animation set starts>
 size: <integer tuple of how big each cell is; no stride is allowed>
 frames: <number of frames>
 delay: <number of seconds, as a fraction of seconds, between frames>
 
 All deriving classes may pull out their own entity-specific values.
 For example animals need different settings than their mob counterparts.
 
 Also note that as of now, entities are rendered using
 immediate-mode rendering, which may be changed in the future
 based on performance hits.
 
 Note: All paths are now floating point positions of either in
 a block (if it is a half block) or an empty space (bellow is solid
 block). This is all done to help the dwarves during run-time movements
 and to simplify block look ups.
 
***************************************************************/

// Inclusion guard
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <Glui2/g2Config.h>
#include <Glui2/g2Images.h>
#include "WorldContainer.h"
#include "EntityPath.h"
#include "DesignationsView.h"
#include "ItemsView.h"
#include "g2ChatController.h"

// Foward declare, as we can't do an inclusion cycle
class Entities;

// Possible entity error states (i.e. can't move into block, 
enum EntityError
{
    EntityError_None,       // No error
    EntityError_Blocked,    // Cannot move into target (solid object)
};

// Entity instruction set
// Operator: what we are doing
static const int EntityOpCount = 4;
enum EntityOp
{
    EntityOp_Idle = 0,  // Stall (no-op)
    EntityOp_MoveTo,    // Move to "Target"
    EntityOp_Face,      // Turn to "Face"
    EntityOP_Break,     // Break a given block (forces the dwarf to face it during the mining process)
    EntityOp_Pickup,    // Take an item from the current position
    EntityOp_Dropoff,   // Drop item off at the current position
    EntityOp_MovePath,  // Move using the given path
};

// Entity instruction set names
static const char EntityOpNames[EntityOpCount][32] =
{
    "Idle",
    "Moving to target",
    "Facing target",
    "Breaking target",
};

// Each instruction can have a series of parameters
// Instruction: with what args / params
// Note: ONLY one argument at a time
struct EntityInstruction
{
    // Operator & data
    EntityOp Operator;
    
    // Union'ed so we save space
    union __Data
    {
        float Idle;
        float Face;
        struct __Pos
        {
            int x, y, z;
        } Pos;
        int ItemID;
        Stack<Vector3<int> >* Path;
    } Data;
};

// Direction in which an entity is facing
// These directions are ordered by 2D-quadrants
enum EntityFacing
{
    EntityFacing_FR = 0,
    EntityFacing_FL,
    EntityFacing_BL,
    EntityFacing_BR,
};

// List of all wearable items / sets
static const int EntityWearablesCount = 4;
static const char EntityWearablesConfig[EntityWearablesCount][32] =
{
    "Leather Armor.cfg",
    "Steel Armor.cfg",
    "Leather Boots.cfg",
    "Steel Boots.cfg",
};
static const dItemType EntityWearablesType[EntityWearablesCount] =
{
    dItem_ArmorChest_Leather,
    dItem_ArmorChest_Steel,
    dItem_ArmorBoots_Leather,
    dItem_ArmorBoots_Steel,
};

// Animation / activity state
enum EntityState
{
    EntityState_Idle,
    EntityState_Moving,
    EntityState_Breaking,
};

class Entity
{
public:
    
    // Constructor and destructor
    Entity(const char* ConfigName);
    ~Entity();
    
    // Allow configuration access
    g2Config* GetConfigFile();
    
    // Instantly move the entity to this location
    void SetPosition(Vector3<int> Pos);
    void SetPosition(Vector3<float> Pos);
    
    // Returns location
    Vector3<float> GetPosition();
    
    // Get the block location we are in (not on)
    Vector3<int> GetPositionBlock();
    
    // Get size (relative to world size)
    float GetSize();
    
    // Get the correct world size (i.e. if the scale is 1, the world width might be 0.9 and height 1)
    Vector2<float> GetWorldSize();
    
    // Get the pixel size
    Vector2<int> GetPixelSize();
    
    // Get the current entity ID
    int GetEntityID();
    
    // Get current sprite-animation index (i.e. which cell are we in?)
    int GetSpriteCellIndex();
    
    // Get a copy of the current movement instructions
    Stack< Vector3<int> > GetMovingPath();
    
    // Render the path
    void SetPathRender(bool RenderPath);
    
    // Get the current path rendering state
    bool GetPathRender();
    
    // True if entity is still breaking; posts the target block we are breaking and the sprite cell index we should be in
    bool GetBreaking(Vector3<int>* Target, int* Step);
    
    // Set the current breaking state, target, and time to break 
    void SetBreaking(Vector3<int> BreakTarget, float BreakTime);
    
    /*** Entity Processing Management ***/
    
    // Add instruction to processing queue
    void AddInstruction(EntityInstruction Instruction);
    
    // Clear all instructions in the queue
    void ClearInstructions();
    
    // Pop off an instruction; if no instruction found, returns false
    // Note that an instruction WILL be removed from the queue if not empty
    bool GetInstruction(EntityInstruction* Instruction);
    
    // Posts the active instruction that is being executed (returns false if no instruction)
    bool GetActiveInstruction(EntityInstruction* Instruction);
    
    // Returns true if there are some instructions to execute; else returns false
    bool HasInstructions();
    
    // Raise an error of the given type
    // Internally will stop AI execution and push a 1-second idle instruction
    void RaiseExecutionError(EntityError Error);
    
    // Return the current error state of the instruction execution
    // Note: Once called, the error state is reset to none until another error is raised
    EntityError GetExecutionError();
    
    /*** Graphics Management ***/
    
    // Set the angle that this entity should face
    void SetFacingAngle(float Theta);
    
    // Get the current facing of the mob
    float GetFacingAngle();
    
    // Get the current angle
    float GetCameraAngle();
    
    // Draw a preview of the sprite on-screen as a 2D element
    // NOTE: This is a 2D, not 3D, rendering and thus should only
    // be used when in 2D mode (i.e. when rendering the GUI)
    // NOTE: This should be overloaded if the deriving class wears
    // any objects (i.e. armor) ontop of the regular sprite)
    virtual void RenderPreview(int x, int y, int width, int height);
    
    // Same as "RenderPreview(...)" but does actual rendering
    void RenderPreview(int x, int y, int width, int height, float srcx, float srcy, float srcwidth, float srcheight, GLuint TextureID = INT_MAX);
    
    /*** Entity Accessors (Sometimes entity-specific functions needed) ***/
    
    // Get the current health
    virtual int GetHealth();
    
    // Get the max health
    virtual int GetMaxHealth();
    
    // Get the dwarf name
    virtual const char* GetName();
    
protected:
    
    /*** Entity Control System for Derived Classes ***/
    
    // Overloaded for AI handling
    virtual void Update(float dT);
    
    // Overloaded for special graphics
    virtual void Render();
    
    /*** Core Entity Functions (NEVER to be overloaded or ignored) ***/
    
    // Update object
    void __Update(float dT);
    
    // Render object
    void __Render(float CameraAngle);
    
    // Update the physics model of our dwarf (i.e. fall, be pushed, etc.)
    void UpdatePhys(float dT);
    
    // Execute as many commandas as possible, or stall if needed
    void Execute(float dT);
    
    // Execute the movement state (only call this if moving)
    void ExecuteMove(float dT);
    
    // Execute the break state (only call this if breaking)
    void ExecuteBreak(float dT);
    
    /*** Helper Functions ***/
    
    // Get the current main world
    WorldContainer* GetWorld();
    
    // Get the current main designations list
    DesignationsView* GetDesignations();
    
    // Get the current main items list
    ItemsView* GetItems();
    
    // Get the current entities list
    Entities* GetEntities();
    
    // Set the current sprite-sheet states (internal sizes, offsets, timers, etc.)
    // States include "idle_Front", "idle_Front", etc..
    void SetBillboardState(const char* State);
    
    // Render the shadow
    void RenderShadow(Vector3<float> pos, float radius);
    
    // Get the current facing direction of the sprite
    // based on it's location and the location of the camera
    EntityFacing GetGlobalFacing();
    
    // Posts information on the wearable sprite details; returns false if not found
    bool GetWearableSprite(dItemType ItemType, float* x, float* y, float* width, float* height, GLuint* TextureID);
    
    /*** Rendering Functions ***/
    
    // Render the sprite based on the given state
    void RenderBillboard(Vector3<float> pos, float outwidth, float outheight, float srcx, float srcy, float srcwidth, float srcheight, float doffset, bool flip, GLuint TextureID = INT_MAX);
    
    /*** Gameplay Data ***/
    
    int Health, MaxHealth;
    
private:
    
    /*** Helper / Misc. Functions ***/
    
    // Turn a given block into a centered-position for an entity
    bool LocalizePosition(Vector3<int> Pos, Vector3<float>* PosOut);;
    
    /*** Data ***/
    
    // The main world handle; so this AI can
    // explore or manipulate it
    WorldContainer* MainWorld;
    
    // Main designations; so this AI knows where what is
    DesignationsView* Designations;
    
    // Main items; so the AI can get/set items
    ItemsView* Items;
    
    // Configuration file
    g2Config* ConfigFile;
    
    // Texture handle and size
    GLuint TextureID;
    int TextureWidth, TextureHeight;
    
    // Size of the mob, relative to the world
    float Size;
    
    // Current location
    Vector3<float> Location;
    
    // Current camera rotation
    float CameraTheta;
    
    // Current mob's face
    float FacingTheta;
    
    // The old facing direction & move state
    EntityFacing OldGlobalFacing;
    bool WasMoving, WasBreaking;
    
    // Unique entity ID
    int EntityID;
    
    // Entity state
    EntityState State, OldState;
    
    /*** Sprite Sheet State Info ***/
    
    // Position of the sprite-sheet animation subset
    Vector2<int> SpritePos;
    
    // Size per cell in the animation (in pixels)
    Vector2<int> SpriteSize;
    
    // Shadow radius
    float ShadowRadius;
    
    // Number of frames in animation
    int SpriteFrames;
    
    // Fraction of second to wait between each cell
    float SpriteDelay;
    
    // Total time for this current animation
    float SpriteTimer;
    
    // Sprite index
    int SpriteIndex;
    
    // Wearable texture ID & config pair; ordered based on global "EntityWearablesConfig"
    g2Config WearablesConfig[EntityWearablesCount];
    GLuint WearablesTextureID[EntityWearablesCount];
    int WearblesTextureWidth[EntityWearablesCount][2];
    
    /*** Animation ***/
    
    // Previous "Source" path
    Vector3<float> AnimationSource;
    
    // If jumping, set to true
    bool IsJumping;
    
    /*** AI / Control ***/
    
    // Access to all other entities
    Entities* WorldEntities;
    
    // Queue of instructions to execute
    Queue< EntityInstruction > InstructionQueue;
    
    // Execution error state
    EntityError ExecutionError;
    
    // Active instruction
    EntityInstruction ActiveInstruction;
    
    // True if currently executing an instruction
    bool IsExecuting;
    
    // Total time this instruction has been executing
    float ExecutionTime;
    
    // Do we render the path?
    bool RenderPath;
    
    // What is the dwarf's movement instructions?
    Stack< Vector3<int> > MovingPath;
    
    // Target we are attempting to move to
    Vector3<int> MovingTarget;
    
    // Current path generation object
    EntityPath* PathPlanner;
    
    /*** Breaking Info. ***/
    
    // Target we are attempting to break
    Vector3<int> BreakingTarget;
    
    // Countdown to the breaking event and the total breaking time
    float BreakingTime, FullBreakingTime;
    
    // The breaking skin texture cell ID
    int BreakingCellIndex;
    
    /*** Misc. ***/
    
    // Mutext to protect the instructions list
    pthread_mutex_t InstructionsLock;
    
    // Declare we are a friend so we can be more easily
    // reached when attempting to update / initialize
    friend class Entities;
    
};

// End of inclusion guard
#endif
