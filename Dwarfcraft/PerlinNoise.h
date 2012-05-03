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
static const int PerlinNoise_Size = 1024;

class PerlinNoise
{
public:
    
    // Initialize with nothing
    PerlinNoise(int octaves, float freq, float amp, const char* Seed);
    
    // Release internals
    ~PerlinNoise();
    
    // Returns a normalized value for the given "map" position, which
    // should also be normalized. This means that giving a set of (0.5, 0.5)
    // returns the [0, 1] value of the middle pixel.
    float perlin_noise_2D(float vec[2]);
	float GetNoise1D(float x);
	float GetNoise2D(float x, float y);
	float GetNoise3D(float x, float y, float z);
    
private:
    
    // Helper funcs
    void normalize2(float v[2]);
    void normalize3(float v[3]);
    
    // Interpolation
    float s_curve(float t) { return t * t * (3.0f - 2.0f * t); }
    float lerp(float t, float a, float b) { return a + t * (b - a); }
    
    // Args
    int   mOctaves;
    float mFrequency;
    float mAmplitude;
    int   mSeed;
    
    // Sample data
    int p[PerlinNoise_Size + PerlinNoise_Size + 2];
    float g3[PerlinNoise_Size + PerlinNoise_Size + 2][3];
    float g2[PerlinNoise_Size + PerlinNoise_Size + 2][2];
    float g1[PerlinNoise_Size + PerlinNoise_Size + 2];
    
    // Random generator
    UtilRand RandGen;
};

// End inclusion guard
#endif
