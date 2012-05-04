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
static const int dBlockType_Count = 33;

// Enumerate block types (must be <= 255; unsigned char)
// As of May 1st, all blocks have been redefined
// Specialy "pseud-blocks" (such as tree trunk tops) are defined in the bottom
enum dBlockType
{
    // Generic naturally occuring
    dBlockType_Air,
    dBlockType_Stone,
    dBlockType_Cobblestone,
    dBlockType_Dirt,
    dBlockType_Bedrock,
    dBlockType_Water,
    dBlockType_Lava,
    dBlockType_Sand,
    dBlockType_Gravel,
    dBlockType_Wood,
    dBlockType_Leaves,
    dBlockType_Grass,
    dBlockType_Bush,
    dBlockType_Flower,
    dBlockType_Mushroom,
    
    // Ores
    dBlockType_CoalOre,
    dBlockType_IronOre,
    dBlockType_SilverOre,
    dBlockType_GoldOre,
    dBlockType_DiamondOre,
    
    // Crafted
    dBlockType_Plank,
    dBlockType_Torch,
    dBlockType_Chest,
    dBlockType_Furnace,
    dBlockType_Door,
    dBlockType_Stairs,
    dBlockType_Glass,
    dBlockType_Ladder,
    
    // Workbenches
    dBlockType_CarpentryBench,
    dBlockType_MasonryBench,
    dBlockType_EngineeringBench,
    dBlockType_KitchenBench,
    dBlockType_SmithingBench,
};

// Matches, for the most part, Minecraft's textures
static Vector2<int> dBlockTexturePos[dBlockType_Count] =
{
    Vector2<int>(0, 0),
    Vector2<int>(1, 0),
    Vector2<int>(0, 1),
    Vector2<int>(2, 0),
    Vector2<int>(11, 7),
    Vector2<int>(13, 12),
    Vector2<int>(13, 14),
    Vector2<int>(2, 1),
    Vector2<int>(3, 1),
    Vector2<int>(4, 1),
    Vector2<int>(4, 3),
    Vector2<int>(8, 5),
    Vector2<int>(15, 0),
    Vector2<int>(12, 0),
    Vector2<int>(13, 1),
    
    Vector2<int>(2, 2),
    Vector2<int>(1, 2),
    Vector2<int>(3, 3),
    Vector2<int>(0, 2),
    Vector2<int>(2, 3),
    
    Vector2<int>(4, 0),
    Vector2<int>(0, 5),
    Vector2<int>(9, 1),
    Vector2<int>(12, 2),
    Vector2<int>(1, 5),
    Vector2<int>(0, 1),
    Vector2<int>(1, 3),
    Vector2<int>(4, 5),
    
    // Benches are not textures in the terrain.png list
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
    Vector2<int>(0, 0),
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

// Non-block, used for designations texture selection
static const Vector2<int> dDesignationTexturePos(0, 16); // Paired with UI

// Non-block, used for the breaking animation
static const Vector2<int> dBreakingTexturePos(0, 15); // Paired with terrain

// Total number of item types in the world
static const int dItemType_Count = 44;

// Items
enum dItemType
{
    // None / nothing
    dItem_None,
    
    // Block equivalents
    dItem_Stone,
    dItem_Cobblestone,
    dItem_Dirt,
    dItem_Bedrock,
    dItem_Sand,
    dItem_Gravel,
    dItem_Wood,
    dItem_Leaves,
    dItem_Grass,
    dItem_Bush,
    dItem_Flower,
    dItem_Mushroom,
    dItem_CoalOre,
    dItem_IronOre,
    dItem_SilverOre,
    dItem_GoldOre,
    dItem_DiamondOre,
    dItem_Plank,
    dItem_Torch,
    dItem_Chest,
    dItem_Furnace,
    dItem_Door,
    dItem_Stairs,
    dItem_Glass,
    dItem_Ladder,
    
    // Smelted materials
    dItem_IronBar,
    dItem_SilverBar,
    dItem_GoldBar,
    dItem_DiamondBar,
    
    // Crafted items (produced)
    dItem_Axe,
    dItem_Pick,
    dItem_Shovel,
    dItem_Hoe,
    dItem_Sword,
    dItem_Bow,
    dItem_Arrow,
    
    // Wearables
    dItem_ChestArmor,
    dItem_PantsArmor,
    dItem_FeetArmor,
    dItem_HelmArmor,
    
    // Furniture
    dItem_Table,
    dItem_Chair,
    dItem_Bed,
};

// Item texture position (which changes based on the item base size)
static Vector2<int> dItemTypeTexturePos[dItemType_Count] = 
{
    Vector2<int>(),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 1),
    Vector2<int>(10, 1),
    Vector2<int>(10, 1),
    Vector2<int>(10, 1),
    Vector2<int>(10, 1),
    Vector2<int>(7, 1),
    Vector2<int>(7, 1),
    Vector2<int>(7, 1),
    Vector2<int>(7, 1),
    Vector2<int>(7, 1),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(11, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(10, 2),
    Vector2<int>(7, 1),
    Vector2<int>(7, 1),
    Vector2<int>(7, 1),
    Vector2<int>(7, 1),
    Vector2<int>(0, 7),
    Vector2<int>(0, 6),
    Vector2<int>(0, 5),
    Vector2<int>(0, 8),
    Vector2<int>(0, 4),
    Vector2<int>(5, 1),
    Vector2<int>(5, 2),
    Vector2<int>(0, 1),
    Vector2<int>(0, 2),
    Vector2<int>(0, 3),
    Vector2<int>(0, 0),
    Vector2<int>(8, 8),
    Vector2<int>(8, 8),
    Vector2<int>(8, 8),
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

// Get the breaking texture
GLuint dGetBreakingTexture(float* x, float* y, float* width, float* height);

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

// Map a block to the associated item
// May return "dItem_None" if no item-block match
dItemType dGetItemFromBlock(dBlock Block);

// Returns true if the given block (like mushrooms) break if the support is removed
bool dBlockCollapses(dBlock Block);

#endif
