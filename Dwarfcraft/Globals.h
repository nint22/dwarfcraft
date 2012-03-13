/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Globals.h
 Desc: Generic global data.
 
***************************************************************/

// Inclusion guard
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "VBuffer.h"
#include <Glui2/glui2.h>

/*** User Keys ***/

// Get the given key settings and set it to the DataOut integer buffer
// If the key is not set, a false is returned, else if valid a true is returned
// If the key is not found, the data is set to the given dafault value
static inline bool GetUserSetting(const char* Group, const char* Key, int* DataOut, int DataDefault)
{
    static bool Initialized = false;
    static g2Config Config;
    if(!Initialized)
        Config.LoadFile("Settings.cfg");
    
    // Set the key and return success state
    if(Config.GetValue(Group, Key, DataOut))
        return true;
    else
    {
        *DataOut = DataDefault;
        return false;
    }
}

/*** Initial Values ***/

// Initial camera values
static const int WorldView_InitialFovy = 60;
static const float WorldView_CameraDist = 20;

// Fraction of seconds needed, when holding a key down, to repeat
static const float KeyStallStart = 0.5f; // How long to hold before the repeating starts
static const float KeyStallDelta = 0.2f; // How fast the reapeating goes

/*** Rendering styles ***/

// Define all major types of rendering
enum GameRender_RenderStyle
{
    GameRender_RenderStyle_Point = 0,
    GameRender_RenderStyle_Lines,
    GameRender_RenderStyle_Fill,
    GameRender_RenderStyle_Textured,
};

/*** Cube Geometry ***/

// Number of floats per vertex entry ((x,y,z)(u,v)(r,g,b))
static const int GameRender_FloatsPerVertex = 8;

// All offsets we are to verify (top, front, back, left, right)
static const Vector3<int> GameRender_FaceOffsets[5] =
{
    Vector3<int>(0, 1, 0),  // Top
    Vector3<int>(1, 0, 0),  // Front
    Vector3<int>(-1, 0, 0), // Back
    Vector3<int>(0, 0, -1), // Left
    Vector3<int>(0, 0, 1),  // Right
};

// Define all normals; indexed in parallel to face offsets
static const Vector3<float> WorldView_Normals[5] =
{
    Vector3<float>(0, 1, 0),
    Vector3<float>(1, 0, 0),
    Vector3<float>(-1, 0, 0),
    Vector3<float>(0, 0, -1),
    Vector3<float>(0, 0, 1),
};

// Define all face geometry as quads; indexed in parallel to face offsets
static const Vector3<float> WorldView_FaceQuads[5][4] =
{
    {
        Vector3<float>(0.0f, 1.0f, 0.0f),
        Vector3<float>(0.0f, 1.0f, 1.0f),
        Vector3<float>(1.0f, 1.0f, 1.0f),
        Vector3<float>(1.0f, 1.0f, 0.0f),
    },
    {
        Vector3<float>(1.0f, 1.0f, 0.0f),
        Vector3<float>(1.0f, 1.0f, 1.0f),
        Vector3<float>(1.0f, 0.0f, 1.0f),
        Vector3<float>(1.0f, 0.0f, 0.0f),
    },
    {
        Vector3<float>(0.0f, 1.0f, 1.0f),
        Vector3<float>(0.0f, 1.0f, 0.0f),
        Vector3<float>(0.0f, 0.0f, 0.0f),
        Vector3<float>(0.0f, 0.0f, 1.0f),
    },
    {
        Vector3<float>(0.0f, 1.0f, 0.0f),
        Vector3<float>(1.0f, 1.0f, 0.0f),
        Vector3<float>(1.0f, 0.0f, 0.0f),
        Vector3<float>(0.0f, 0.0f, 0.0f),
    },
    {
        Vector3<float>(1.0f, 1.0f, 1.0f),
        Vector3<float>(0.0f, 1.0f, 1.0f),
        Vector3<float>(0.0f, 0.0f, 1.0f),
        Vector3<float>(1.0f, 0.0f, 1.0f),
    }
};

// A true 3D model used in-game, like a mushroom or caravan, or 
struct WorldView_Model
{
    // Model's level position
    int x, z;
    
    // Model VBO
    VBuffer* ModelData;
};

// A helper data structure that does the offsets for a 3x3 grid (including the origin)
static const Vector2<int> WorldView_Grid3Offsets[9] =
{
    Vector2<int>(-1, 1),
    Vector2<int>(0, 1),
    Vector2<int>(1, 1),
    Vector2<int>(-1, 0),
    Vector2<int>(0, 0),
    Vector2<int>(1, 0),
    Vector2<int>(-1, -1),
    Vector2<int>(0, -1),
    Vector2<int>(1, -1),
};

// A helper data structure that gives the offsets for an above/bottom/left/right/front/back set
static const int AdjacentOffsetsCount = 6;
static const Vector3<int> AdjacentOffsets[AdjacentOffsetsCount] = {
    Vector3<int>(1, 0, 0),
    Vector3<int>(-1, 0, 0),
    Vector3<int>(0, 1, 0),
    Vector3<int>(0, -1, 0),
    Vector3<int>(0, 0, 1),
    Vector3<int>(0, 0, -1),
};

/** Icon Set for the GUI **/

// Enumeration of all icons
// Note that the first elements have the same index (i.e. parallel
// to) the enumeration "DesignationType"
static const int IconTypeCount = 22;
enum IconType
{
    // Construction
    IconType_Mine = 0, // Just start so we match "DesignationType_Mine = 0"
    IconType_Fill,
    IconType_Flood,
    
    // Storage
    IconType_Rubbish,
    IconType_Food,
    IconType_Crafted,
    IconType_RawResources, // Wood, stones, etc.
    IconType_Ingots,
    IconType_Grave,
    
    // Collection
    IconType_Farm,
    IconType_Wood,
    IconType_Forage,
    
    // Military flags
    IconType_Protect,
    IconType_Barracks,
    IconType_Hall,
    IconType_Armory,
    
    // Not parallel to "DesignationType"
    
    // Global menu
    IconType_Construct,
    IconType_Storage,
    IconType_Collect,
    IconType_Military,
    
    // Accept or cancel
    IconType_Accept,
    IconType_Cancel,
};

// The names of each icon
static const char IconTypeNames[IconTypeCount][32] =
{
    "IconMine",
    "IconFill",
    "IconFlood",
    "IconRubbish",
    "IconFood",
    "IconCrafted",
    "IconRawResources",
    "IconIngots",
    "IconGrave",
    "IconFarm",
    "IconWood",
    "IconForage",
    "IconProtect",
    "IconBarracks",
    "IconHall",
    "IconArmory",
    "IconConstruct",
    "IconStorage",
    "IconCollect",
    "IconMilitary",
    "IconAccept",
    "IconCancel",
};

// Internal helper function to give the Glui2 name based on the icon type
static inline const char* GetIconName(IconType Type)
{
    return IconTypeNames[(int)Type];
}

// End of inclusion guard
#endif
