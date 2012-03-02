/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: WorldGenerator.h/cpp
 Desc: Generate a new world; creating trees, scenery, etc. Based
 on several 2D-perlin maps in combination.
 
***************************************************************/

// Inclusion guard
#ifndef __WORLDGENERATOR_H__
#define __WORLDGENERATOR_H__

#include "WorldContainer.h"
#include "Stack.h"
#include "Vector3.h"
#include "PerlinNoise.h"
#include "PlasmaNoise.h"

class WorldGenerator
{
public:
    
    // Standard constructor and destructor; prepares the generation system
    WorldGenerator();
    ~WorldGenerator();
    
    // Generate a world, filling the given (assumed allocated) world container
    void Generate(WorldContainer* WorldData, const char* Seed);
    
protected:
    
    /*** Scene Generation Functions ***/
    
    // Place a tree at the given location
    void PlaceTree(int x, int z);
    
    // Place a blob of given blocks at a target location
    // This does a randomized fill of adjacent areas
    // For each step, place a block and randomly may recursivly
    // call the same function for other blocks.
    // When steps are 0, nothing is places. The bigger the blob
    // you want, the bigger the step count you want; default is 2
    // Note that this will overwrite any block EXCEPT air (so we
    // don't have "poping out" blocks of walls)
    void PlaceBlob(int x, int y, int z, dBlockType type, int steps = 2);
    
    // Place foliage at the given block location
    void PlaceFoliage(int x, int z);
    
private:
    
    // Get the height of the block of the top-most layer (earth-top)
    float GetSurfaceHeight(PerlinNoise* TerrainHeight, int x, int z);
    
    // Get the height of the second layer (soil)
    float GetDirtHeight(PerlinNoise* TerrainHeight, int x, int z);
    
    // Get the height of the third layer (stone)
    float GetStoneHeight(PerlinNoise* TerrainHeight, int x, int z);
    
    // Random number generator
    UtilRand* Random;
    
    // The main world we are working with in this instance
    // Internal reference
    WorldContainer* WorldData;
};

// End inclusion guard
#endif
