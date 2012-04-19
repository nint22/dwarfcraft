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
// Note this INCLUDES the middle position (which could be a half block)
static const int AdjacentOffsetsCount = 7;
static const Vector3<int> AdjacentOffsets[AdjacentOffsetsCount] = {
    Vector3<int>(0, 0, 0),
    Vector3<int>(1, 0, 0),
    Vector3<int>(-1, 0, 0),
    Vector3<int>(0, 1, 0),
    Vector3<int>(0, -1, 0),
    Vector3<int>(0, 0, 1),
    Vector3<int>(0, 0, -1),
};

/** Icon Set for the GUI **/

// Enumeration of all icons
// These are placed in an 8-per-row, next-row-down order
// The starting position is a key-tuble value pair in "UITheme.cfg" called "IconsList"
static const int IconTypeCount = 27;
enum IconType
{
    Icon_RootMenu_Build,
    Icon_RootMenu_Designations,
    Icon_RootMenu_Stockpiles,
    Icon_RootMenu_Zones,
    Icon_BuildMenu_Farm,
    Icon_BuildMenu_Architecture,
    Icon_BuildMenu_Workshops,
    Icon_BuildMenu_Furniture,
    Icon_BuildMenu_Mechanical,
    Icon_DesignationMenu_Fell,
    Icon_DesignationMenu_Forage,
    Icon_DesignationMenu_Mine,
    Icon_DesignationMenu_Fill,
    Icon_DesignationMenu_Flood,
    Icon_StockpilesMenu_Rubbish,
    Icon_StockpilesMenu_Food,
    Icon_StockpilesMenu_Crafted,
    Icon_StockpilesMenu_Equipment,
    Icon_StockpilesMenu_RawResources,
    Icon_StockpilesMenu_Ingots,
    Icon_StockpilesMenu_Graves,
    Icon_StockpilesMenu_Wood,
    Icon_ZonesMenu_Hall,
    Icon_ZonesMenu_Pen,
    Icon_ZonesMenu_Defend,
    Icon_Accept,
    Icon_Cancel,
};

/*** User Interface Icons ***/

// Root menu:
static const int UI_RootMenuCount = 4;
enum UI_RootMenu
{
    UI_RootMenu_Build,          // Place an item, wall, etc.
    UI_RootMenu_Designations,   // Manipulate the world
    UI_RootMenu_Stockpiles,     // Storage
    UI_RootMenu_Zones,          // Places for dwarves, defense, etc
};
static const char UI_RootMenuNames[UI_RootMenuCount][32] =
{
    "Build",
    "Designations",
    "Stockpiles",
    "Zones",
};

// Build menu:
static const int UI_BuildMenuCount = 5;
enum UI_BuildMenu
{
    UI_BuildMenu_Farm,          // Simple farm
    UI_BuildMenu_Architecture,  // Stairs, floors, walls, etc.
    UI_BuildMenu_Workshops,     // Masonry, woodshop, etc.
    UI_BuildMenu_Furniture,     // Chairs, doors, etc.
    UI_BuildMenu_Mechanical,    // Levers, gates, etc.
};
static const char UI_BuildMenuNames[UI_BuildMenuCount][32] =
{
    "Farm",
    "Architecture",
    "Workshops",
    "Furniture",
    "Mechanical",
};

// Designations menu:
static const int UI_DesignationsMenuCount = 5;
enum UI_DesignationMenu
{
    UI_DesignationMenu_Fell,
    UI_DesignationMenu_Forage,
    UI_DesignationMenu_Mine,
    UI_DesignationMenu_Fill,
    UI_DesignationMenu_Flood,   // Flood with water (via buckets)
};
static const char UI_DesignationsMenuNames[UI_DesignationsMenuCount][32] =
{
    "Fell Trees",
    "Forage",
    "Mine",
    "Fill",
    "Flood",
};

// Stockpiles menu:
static const int UI_StockpilesMenuCount = 8;
enum UI_StockpilesMenu
{
    UI_StockpilesMenu_Rubbish,
    UI_StockpilesMenu_Food,
    UI_StockpilesMenu_Crafted,
    UI_StockpilesMenu_Equipment,
    UI_StockpilesMenu_RawResources, // Wood, stones, etc.
    UI_StockpilesMenu_Ingots,
    UI_StockpilesMenu_Graves,
    UI_StockpilesMenu_Wood,
};
static const char UI_StockpilesMenuNames[UI_StockpilesMenuCount][32] =
{
    "Rubbish",
    "Food",
    "Crafted",
    "Equipment",
    "Raw Resources",
    "Ignots",
    "Graves",
    "Wood",
};

// Zones:
static const int UI_ZonesMenuCount = 3;
enum UI_ZonesMenu
{
    UI_ZonesMenu_Hall,              // All idle dwarves go here
    UI_ZonesMenu_Pen,               // Animals
    UI_ZonesMenu_Defend,            // Any enimies enter here, we attack
};
static const char UI_ZonesMenuNames[UI_ZonesMenuCount][32] =
{
    "Hall",
    "Animal Pen",
    "Defend",
};

// End of inclusion guard
#endif
