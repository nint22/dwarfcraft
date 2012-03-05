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

// End of inclusion guard
#endif
