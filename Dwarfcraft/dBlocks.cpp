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
    if(BlockType <= 1 || BlockType >= dBlockType_Count)
        return;
    
    // Get the appropriate base-face location
    Vector2<int> Origin = dBlockTexturePos[BlockType];
    *x = float(Origin.x) / float(TextureWidth);
    *y = float(Origin.y) / float(TextureHeight);
    *width = float(TileSize) / float(TextureWidth);
    *height = float(TileSize) / float(TextureHeight);
    
    if(rotations != NULL)
        *rotations = 0;
    
    // Default color to white, unless the texture is exceptional
    if(color != NULL)
    {
        *color = Vector3<float>(1, 1, 1);
        if(BlockType == dBlockType_Leaves)
            *color = Vector3<float>(0.372f, 0.623f, 0.207f);
    }
    
    // Dirt has different sides and tops IF it is 1 (grass), or 2 (snow)
    if(BlockType == dBlockType_Dirt && Block.GetMeta() == 1)
    {
        // Set the top to grass
        if(Face == dBlockFace_Top)
            *x += 2 * *width;
        
        // Side grass
        else if(Face == dBlockFace_Left || Face == dBlockFace_Right || Face == dBlockFace_Back || Face == dBlockFace_Front)
            *x += *width;
    }
    
    // Snow
    else if(BlockType == dBlockType_Dirt && Block.GetMeta() == 2)
    {
        // Set the top to grass
        if(Face == dBlockFace_Top)
            *x += 4 * *width;
        
        // Side grass
        else if(Face == dBlockFace_Left || Face == dBlockFace_Right || Face == dBlockFace_Back || Face == dBlockFace_Front)
            *x += 3 * *width;
    }
    
    // Check special cases
    else if(BlockType == dBlockType_Wood && Face == dBlockFace_Top)
    {
        dGetBlockTexture(dBlock(dBlockType_Wood_Top), Face, x, y, width, height, color, rotations);
    }
    else if(BlockType == dBlockType_StoneSlab && Face == dBlockFace_Top)
    {
        dGetBlockTexture(dBlock(dBlockType_StoneSlab_Top), Face, x, y, width, height, color, rotations);
    }
    // Grass has 7 total tiles (grows x+)
    else if(BlockType == dBlockType_Grass && Block.GetMeta() > 0 && Block.GetMeta() <= 6)
    {
        // Offset based on valid meta
        *x += *width * Block.GetMeta();
    }
    // Mushroom has 2 total tiles (grows x+)
    else if(BlockType == dBlockType_Mushroom && Block.GetMeta() > 0 && Block.GetMeta() <= 1)
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
    
    // If any of the plants...
    if(BlockType == dBlockType_Grass || BlockType == dBlockType_Bush || BlockType == dBlockType_Mushroom || BlockType == dBlockType_Caravan)
        return true;
    else
        return false;
}

bool dIsSolid(dBlock Block)
{
    // Simplify accessor
    dBlockType BlockType = Block.GetType();
    
    // Only return true if solid
    if(BlockType == dBlockType_Air ||
       BlockType == dBlockType_Grass || BlockType == dBlockType_Bush ||
       BlockType == dBlockType_Mushroom || BlockType == dBlockType_Leaves)
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
