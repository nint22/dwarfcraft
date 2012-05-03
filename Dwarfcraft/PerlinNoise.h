/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: PerlinNoise.h/cpp
 Desc: A simple 2D perlon noise implementation based on Perlin's
 own major implementation: http://mrl.nyu.edu/~perlin/doc/oscar.html
 
 Noise is always generated as a normalized value in [0, 1] and
 can be generated in 1, 2, and 3 dimensions.
 
 Easier to understand explanation (but code not derived):
 http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
 
 Current code is based off of a much more clean-cut implementation:
 http://www.dreamincode.net/forums/topic/66480-perlin-noise/
 
***************************************************************/

// Standard includes
#include <stdlib.h>
#include <math.h>
#include "MUtil.h"

// Inclusion guard
#ifndef __PERLINNOISE_H__
#define __PERLINNOISE_H__

// Table size
static const int PerlinNoise_Size = 256;

class PerlinNoise
{
public:
    
    // Initialize with nothing
    PerlinNoise(const char* Seed);
    
    // Release internals
    ~PerlinNoise();
    
    // Returns a normalized value for the given "map" position, which
    // should also be normalized. This means that giving a set of (0.5, 0.5)
    // returns the [0, 1] value of the middle pixel.
	float GetNoise1D(float x);
	float GetNoise2D(float x, float y);
	float GetNoise3D(float x, float y, float z);
    
private:
    
	// Permutation table
	unsigned char p[PerlinNoise_Size];
    
	// Gradients
	float gx[PerlinNoise_Size];
	float gy[PerlinNoise_Size];
	float gz[PerlinNoise_Size];
    
    // Random generator
    UtilRand RandGen;
};

// End inclusion guard
#endif
