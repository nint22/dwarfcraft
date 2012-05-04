/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "dBlocks.h"

GLuint dGetTerrainTextureID(int* Width, int* Height, int* TileSize)
{
    // Internal height, width, and texture ID
    static GLuint TextureID = UINT_MAX;
    static int TextureWidth = -1;
    static int TextureHeight = -1;
    static int TextureTileSize = -1;
    
    // Do initial loading of all terrain / texture data
    if(TextureID == UINT_MAX)
    {
        // Load the terrain cfg file
        g2Config TerrainConfig;
        TerrainConfig.LoadFile("Terrain.cfg");
        
        // Save the tile size
        TerrainConfig.GetValue("General", "TileSize", &TextureTileSize);
        
        // Get and load the the terrain textures
        // TODO: use different seasons in the future
        char* str;
        TerrainConfig.GetValue("General", "Summer", &str);
        TextureID = g2LoadImage(str, &TextureWidth, &TextureHeight, NULL, false, false);
        
        // Convert all the grid positions into texture positions
        for(int i = 0; i < dBlockType_Count; i++)
        {
            dBlockTexturePos[i].x *= TextureTileSize;
            dBlockTexturePos[i].y *= TextureTileSize;
        }
        // All done!
    }
    
    // Post-back as needed
    if(Width != NULL)
        *Width = TextureWidth;
    if(Height != NULL)
        *Height = TextureHeight;
    if(TileSize != NULL)
        *TileSize = TextureTileSize;
    return TextureID;
}

void dGetBlockTexture(dBlock Block, dBlockFace Face, float* x, float* y, float* width, float* height, Vector3<float>* color, int* rotations)
{
    // Get the total width and height
    int TextureWidth, TextureHeight, TileSize;
    dGetTerrainTextureID(&TextureWidth, &TextureHeight, &TileSize);
    
    // Get block type
    dBlockType BlockType = Block.GetType();
    
    // Does this block type exist, is none, or air? Just ignore..
    if(BlockType <= dBlockType_Air || BlockType >= dBlockType_Count)
        return;
    
    // Normalize terrain texture
    Vector2<int> Origin = dBlockTexturePos[BlockType];
    *x = float(Origin.x) / float(TextureWidth);
    *y = float(Origin.y) / float(TextureHeight);
    *width = float(TileSize) / float(TextureWidth);
    *height = float(TileSize) / float(TextureHeight);
    
    // Any desired texture rotations?
    if(rotations != NULL)
        *rotations = 0;
    
    // Default color to white, unless the texture is exceptional
    if(color != NULL)
        *color = Vector3<float>(1, 1, 1);
    
    // Dirt has different sides and tops IF it is 1 (grass), or 2 (snow)
    if(BlockType == dBlockType_Dirt && Block.GetMeta() == 1)
    {
        // Set the top to grass (two faces left), else is grass (one face right)
        if(Face == dBlockFace_Top)
        {
            if(color != NULL)
                *color = Vector3<float>(0.41, 0.66, 0.25);
            *x -= 2 * *width;
        }
        else
            *x += *width;
    }
    
    // Snow
    else if(BlockType == dBlockType_Dirt && Block.GetMeta() == 2)
    {
        // Set the top to snow (4 faces down), else is snow side (2 faces right, 4 down)
        if(Face == dBlockFace_Top)
            *y += 4 * *width;
        else
        {
            *x += 2 * *width;
            *y += 4 * *width;
        }
    }
    
    // Check special cases (trunk, face right)
    else if(BlockType == dBlockType_Wood && Face == dBlockFace_Top)
    {
        if(Face == dBlockFace_Top)
            *x += *width;
    }
    // Grass has 7 total tiles (grows x+)
    else if(BlockType == dBlockType_Grass && Block.GetMeta() > 0 && Block.GetMeta() <= 6)
    {
        // Offset based on valid meta
        *x += *width * Block.GetMeta();
    }
}

GLuint dGetItemTextureID(int* Width, int* Height, int* TileSize)
{
    // Internal height, width, and texture ID
    static GLuint TextureID = UINT_MAX;
    static int TextureWidth = -1;
    static int TextureHeight = -1;
    static int TextureTileSize = -1;
    
    // Do initial loading of all terrain / texture data
    if(TextureID == UINT_MAX)
    {
        // Load the terrain cfg file
        g2Config TerrainConfig;
        TerrainConfig.LoadFile("Terrain.cfg");
        
        // Save the tile size
        TerrainConfig.GetValue("General", "ItemSize", &TextureTileSize);
        
        // Get and load the the terrain textures
        // TODO: use different seasons in the future
        char* str;
        TerrainConfig.GetValue("General", "Items", &str);
        TextureID = g2LoadImage(str, &TextureWidth, &TextureHeight, NULL, false, false);
        
        // Convert all the grid positions into texture positions
        for(int i = 0; i < dItemType_Count; i++)
        {
            dItemTypeTexturePos[i].x *= TextureTileSize;
            dItemTypeTexturePos[i].y *= TextureTileSize;
        }
        // All done!
    }
    
    // Post-back as needed
    if(Width != NULL)
        *Width = TextureWidth;
    if(Height != NULL)
        *Height = TextureHeight;
    if(TileSize != NULL)
        *TileSize = TextureTileSize;
    return TextureID;
}

void dGetItemTexture(dItemType Item, float* x, float* y, float* width, float* height)
{
    // Get the total width and height
    int TextureWidth, TextureHeight, TileSize;
    dGetItemTextureID(&TextureWidth, &TextureHeight, &TileSize);
    
    // Post the correct coordinates
    Vector2<int> Origin = dItemTypeTexturePos[Item];
    *x = float(Origin.x) / float(TextureWidth);
    *y = float(Origin.y) / float(TextureHeight);
    *width = float(TileSize) / float(TextureWidth);
    *height = float(TileSize) / float(TextureHeight);
}

GLuint dGetBreakingTexture(float* x, float* y, float* width, float* height)
{
    // Get the texture ID
    int TextureWidth, TextureHeight, TileSize;
    GLuint TextureID = dGetItemTextureID(&TextureWidth, &TextureHeight, &TileSize);
    
    // Find the offset and normalize the width and height
    *width = (float)TileSize/(float)TextureWidth;
    *height = (float)TileSize/(float)TextureHeight;
    *x = dBreakingTexturePos.x * *width;
    *y = dBreakingTexturePos.y * *height;
    
    // Done!
    return TextureID;
}

bool dAdjacentCheck(dBlock Source, dBlock Adjacent)
{
    // If adjacent is air, always render source
    if(Adjacent.GetType() == dBlockType_Air)
        return true;
    
    // If the adjacent has special geometry, always render source
    else if(dHasSpecialGeometry(Adjacent))
        return true;
    
    // If source is whole and the adjacent is a half block, always render source
    else if(Source.IsWhole() && !Adjacent.IsWhole())
        return true;
    
    // Else, nothing makes us visible, hide the source surface
    else
        return false;
}

bool dHasSpecialGeometry(dBlock Block)
{
    // Simplify accessor
    dBlockType BlockType = Block.GetType();
    
    // Special models (Code is written this way to be simple)
    if(BlockType == dBlockType_Mushroom || BlockType == dBlockType_Torch || BlockType == dBlockType_Chest ||
       BlockType == dBlockType_Furnace || BlockType == dBlockType_Door || BlockType == dBlockType_Stairs || BlockType == dBlockType_Ladder ||
       BlockType == dBlockType_CarpentryBench || BlockType == dBlockType_MasonryBench || BlockType == dBlockType_EngineeringBench ||
       BlockType == dBlockType_KitchenBench || BlockType == dBlockType_SmithingBench)
        return true;
    
    // Else, no match
    return false;
}

bool dIsSolid(dBlock Block)
{
    // If solid
    // Simplify accessor
    dBlockType BlockType = Block.GetType();
    
    // Only return true if solid
    if(dHasSpecialGeometry(Block) ||
       BlockType == dBlockType_Air || BlockType == dBlockType_Grass || BlockType == dBlockType_Bush ||
       BlockType == dBlockType_Flower || BlockType == dBlockType_Mushroom || BlockType == dBlockType_Leaves)
        return false;
    else
        return true;
}

float dGetBreakTime(int ItemID, dBlock Block)
{
    // For now, just return 3 seconds
    // TODO: Compute the breaking time
    return 3.0f;
}

dItemType dGetItemFromBlock(dBlock Block)
{
    // For now, we do some simple matches
    switch(Block.GetType())
    {
        case dBlockType_Stone:      return dItem_Stone;
        case dBlockType_Cobblestone:return dItem_Cobblestone;
        case dBlockType_Dirt:       return dItem_Dirt;
        case dBlockType_Bedrock:    return dItem_Bedrock;
        case dBlockType_Sand:       return dItem_Sand;
        case dBlockType_Gravel:     return dItem_Gravel;
        case dBlockType_Wood:       return dItem_Wood;
        case dBlockType_Leaves:     return dItem_Leaves;
        case dBlockType_Grass:      return dItem_Grass;
        case dBlockType_Bush:       return dItem_Bush;
        case dBlockType_Flower:     return dItem_Flower;
        case dBlockType_Mushroom:   return dItem_Mushroom;
        case dBlockType_CoalOre:    return dItem_CoalOre;
        case dBlockType_IronOre:    return dItem_IronOre;
        case dBlockType_SilverOre:  return dItem_SilverOre;
        case dBlockType_GoldOre:    return dItem_GoldOre;
        case dBlockType_DiamondOre: return dItem_DiamondOre;
        default: break;
    }
    
    return dItem_None;
}

bool dBlockCollapses(dBlock Block)
{
    // If not a solid block, it collapses
    return !dIsSolid(Block);
}
