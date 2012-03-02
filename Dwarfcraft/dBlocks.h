/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: dBlocks.cpp
 Desc: The base definition of all block IDs and the block data
 structure itself. Also defines all items.
 
***************************************************************/

// Inclusion guard
#ifndef __DBLOCKS_H__
#define __DBLOCKS_H__

#include "MUtil.h"
#include "Vector2.h"
#include "Vector3.h"

// Total number of block types in the world
static const int dBlockType_Count = 22;

// Enumerate block types (must be <= 255; unsigned char)
// Note that dBlock_None is default to -1 since it us "unknown"
// Also note, some of these aren't normal blocks, but are used
// to map out specialty sides (i.e. a tree trunk uses a different top texture)
enum dBlockType
{
    // Testing types
    dBlockType_Air = 0,
    dBlockType_Gravel,
    dBlockType_SmoothStone,
    dBlockType_RoughStone,
    dBlockType_Dirt,
    dBlockType_Sand,
    dBlockType_Wood,
    dBlockType_Wood_Top,
    dBlockType_Coal,
    dBlockType_StoneSlab,
    dBlockType_StoneSlab_Top,
    dBlockType_Torch,
    dBlockType_Lava,
    dBlockType_Leaves,
    dBlockType_Grass,
    dBlockType_Bush,
    dBlockType_Mushroom,
    dBlockType_Border,
    dBlockType_Breaking,
    dBlockType_Caravan
};

// Enumerate a block's facing direction
enum dBlockFace
{
    dBlockFace_Top,
    dBlockFace_Bottom,
    dBlockFace_Left,
    dBlockFace_Right,
    dBlockFace_Front,
    dBlockFace_Back,
};

// The starting location of each block
// Note that these are grid positions; not pixel positions, upon initialization
static Vector2<int> dBlockTexturePos[dBlockType_Count] =
{
    Vector2<int>(0, 0),
    Vector2<int>(3, 1),
    Vector2<int>(1, 0),
    Vector2<int>(0, 1),
    Vector2<int>(2, 0),
    Vector2<int>(2, 1),
    Vector2<int>(4, 1),
    Vector2<int>(5, 1),
    Vector2<int>(2, 2),
    Vector2<int>(5, 0),
    Vector2<int>(6, 0),
    Vector2<int>(0, 5),
    Vector2<int>(14, 14),
    Vector2<int>(4, 8),
    Vector2<int>(8, 5),
    Vector2<int>(15, 4),
    Vector2<int>(12, 1),
    Vector2<int>(0, 0),
    Vector2<int>(0, 15),
};

// Non-block, used for designations texture selection
static const Vector2<int> dDesignationTexturePos(0, 12);

// Total number of item types in the world
static const int dItemType_Count = 63;

// Item declaration (similar to minecraft items)
enum dItemType
{
    // None / nothing
    dItem_None,
    
    // Crafted items (produced)
    dItem_Axe,
    dItem_Pickaxe,
    dItem_Shovel,
    dItem_Hoe,
    dItem_SkinningKnife,
    dItem_ButchersKnife,
    dItem_Sheers,
    dItem_FishingNet,
    dItem_FishingRod,
    dItem_Sword,
    dItem_Shield,
    dItem_Polearm,
    dItem_Bow,
    dItem_Arrow,
    dItem_Daggers,
    dItem_HeavySword,
    
    // Todo: mined materials
    dItem_Coal,
    dItem_Dirt,
    dItem_Sand,
    
    // Todo: smelted bars
    dItem_Iron,
    dItem_Gold,
    
    // Todo: Benches we can place
    dItem_CarpentryBench,
    dItem_StonecraftBench,
    dItem_EngineeringBench,
    dItem_CookingBench,
    dItem_ForgeBench,
    dItem_WeaponBench,
    dItem_ArmorBench,
    
    // Todo: power sources
    dItem_Furnace,
    dItem_WaterMill,
    dItem_MagmaRod,
    
    // Todo: light-emitting items
    dItem_Torch,
    dItem_Lantern,
    
    // Todo: mechanisms
    dItem_Wire,
    dItem_Repeater,
    dItem_Switch,
    dItem_LogicalAND,
    dItem_LogicalOR,
    dItem_LogicalNOT,
    dItem_Pump,
    dItem_FloodGate,
    dItem_Rail,
    dItem_Door,
    dItem_MechDoor,
    dItem_PressurePlate,
    dItem_TrapSpike,
    dItem_TrapMechSpike,
    dItem_AnimalAttractor,
    dItem_MobAttractor,
    
    // Todo: storage
    dItem_Crate,
    dItem_Barrel,
    dItem_Bucket,
    
    // Todo: furniture
    dItem_Table,
    dItem_Chair,
    dItem_Bed,
    dItem_Cup,
    dItem_Casket,
    
    // Todo: Wearable
    dItem_ArmorChest_Leather,
    dItem_ArmorChest_Steel,
    dItem_ArmorBoots_Leather,
    dItem_ArmorBoots_Steel,
    
    // Todo: foods
};

// Item texture position (which changes based on the item base size)
static Vector2<int> dItemTypeTexturePos[dItemType_Count] = 
{
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(13, 3), // Coal
    Vector2<int>(10, 3), // Dirt
    Vector2<int>(11, 3), // Sand
    Vector2<int>(14, 3), // Iron
    Vector2<int>(12, 3), // Gold
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
};

// Define item structure; item ID, quality, and current endurance
struct dItem
{
    // Default constructor
    dItem()
    {
        Type = dItem_None;
        Endurance = 0;
    }
    
    // Short-hand constructor
    dItem(dItemType ItemType, unsigned char Quality = 0)
    {
        Type = ItemType;
        Endurance = Quality;
    }
    
    // Data
    dItemType Type;
    unsigned char Endurance;
    
    // TODO: Each item should have some more data, such as a crafting
    // story and crafter name
};

// Define a block structure
struct dBlock
{
    // High-bit flag (internal)
    static const unsigned char __dBlock_HighBit = 0x80;
    
    // Default constructor (no meta, whole block)
    // High bit reservered for whole / half-block state
    dBlock()
    {
        blockType = dBlockType_Air;
        blockMeta = __dBlock_HighBit;
    }
    
    // Short-hand constructor (given meta, whole block)
    // Note that the meta has to be at most 7-bits (of a uchar)!
    // High bit reservered for whole / half-block state
    dBlock(dBlockType blockID, unsigned char meta = 0)
    {
        blockType = (unsigned char)blockID;
        blockMeta = meta | __dBlock_HighBit;
    }
    
    // Set to whole block (true) or half block (false)
    void SetWhole(bool IsWhole)
    {
        // Default to high (full block)
        blockMeta |= __dBlock_HighBit;
        
        // Else, high-bit to off (just XOR it out)
        if(!IsWhole)
            blockMeta ^= __dBlock_HighBit;
    }
    
    // Helper function to get the height (0-3; which is an alias to sizes of 1/4, 1/2, 3/4, and 1
    bool IsWhole()
    {
        // Mask just to see the high-but, then shift it down to either 0x00 or 0x01
        return bool((blockMeta & __dBlock_HighBit) >> 7);
    }
    
    // Set the meta
    void SetMeta(unsigned char Meta)
    {
        // Only set to the lower 7 bits
        blockMeta |= Meta & (~__dBlock_HighBit);
    }
    
    // Get the meta
    unsigned char GetMeta()
    {
        // Retrieve only the lower 7 bits
        return (blockMeta & (~__dBlock_HighBit));
    }
    
    // Set the block type
    void SetType(dBlockType type)
    {
        blockType = (unsigned char)type;
    }
    
    // Get the block type
    dBlockType GetType()
    {
        return (dBlockType)blockType;
    }
    
private:
    
    // Block type (ID)
    unsigned char blockType;
    
    // Meta data (direction, quantity, etc..)
    // Lower 6 bits (0,1,2,4, 8, 16) used for meta-ID, while
    // the two high bits (32, 64) are reserved for height
    // declaration. Hight now varies from 0 to 3, thus four full tile sizes.
    unsigned char blockMeta;
    
};

// Get the target texture (allocates it interally if not yet allocated)
GLuint dGetTerrainTextureID(int* Width = NULL, int* Height = NULL, int* TileSize = NULL);

// Returns the texture coordinates of a cube; may do internal rotation so the coordinates are correct without the UV indices having to change
void dGetBlockTexture(dBlock Block, dBlockFace Face, float* x, float* y, float* width, float* height, Vector3<float>* color = NULL, int* rotations = NULL);

// Get the item texture (allocates it internally if not yet allocates)
GLuint dGetItemTextureID(int* Width = NULL, int* Height = NULL, int* TileSize = NULL);

// Returns the texture coordinates of an item
void dGetItemTexture(dItemType Item, float* x, float* y, float* width, float* height);

// Returns true if the surface from source touching adjacent should be rendered
// For example, if the given source block is dir, and the adjacent is air, all
// source surfaces should be be rendered. Another example: If the source is solid
// and the given block is grass, then we should render the source surfaces (because grass is see-through)
bool dAdjacentCheck(dBlock Source, dBlock Adjacent);

// Returns true if the given block is / has special geometry
// Commonly used in foilage / blants
bool dHasSpecialGeometry(dBlock Block);

// Returns true if the given block is solid
bool dIsSolid(dBlock Block);

// Returns the amount of seconds (as a fraction) of the time it takes to use the given tool against the given block type
float dGetBreakTime(int ItemID, dBlock Block);

#endif
