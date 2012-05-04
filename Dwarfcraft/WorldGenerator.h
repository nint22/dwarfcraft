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

/*** External Includes ***/

// Voronoi gen. tools
#include "Voronoi.h"
#include "VPoint.h"
using namespace vor;

// Testing
#include "EasyBMP/EasyBMP.h"
#include "EasyBmp/EasyBMP_Geometry.h"

// Define biomes
static const int BiomeTypesCount = 6;
enum BiomeTypes                 // Elevation        Moister
{
    BiomeTypes_Snow,            // High             High
    BiomeTypes_Tundra,          // High             Low
    BiomeTypes_Grasslands,      // Med              Low
    BiomeTypes_EvergreenForest, // Med              High
    BiomeTypes_TropicalForest,  // Low              High
    BiomeTypes_Desert,          // Low              Low
};

class WorldGenerator
{
public:
    
    // Standard constructor and destructor; prepares the generation system
    WorldGenerator();
    ~WorldGenerator();
    
    // Generate a world, filling the given (assumed allocated) world container
    void Generate(WorldContainer* WorldData, const char* Seed);
    
protected:
    
    // Fill all white spaces that touch this pixel and all other pixels with the given color
    // Uses flood-fill algorithm
    void FloodFill(BMP& Image, int x, int y, RGBApixel Color);
    
    // True if the given point matches the target color
    bool IsColor(BMP& Image, VPoint* Point, RGBApixel TargetColor);
    
private:
    
    // Random number generator
    UtilRand* Random;
    
    // The main world we are working with in this instance
    // Internal reference
    WorldContainer* WorldData;
};

// End inclusion guard
#endif
