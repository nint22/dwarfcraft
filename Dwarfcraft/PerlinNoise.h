/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: PerlinNoise.h/cpp
 Desc: A simple 2D perlon noise implementation based on Perlin's
 own major implementation: http://mrl.nyu.edu/~perlin/doc/oscar.html
 
 Noise is always generated on a 0-255 (unisnged char) depth ontop
 of the given width x height buffer.
 
 Easier to understand explanation:
 http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
 
 Current code is based off of a much more clean-cut implementation:
 http://www.dreamincode.net/forums/topic/66480-perlin-noise/
 
***************************************************************/

// Standard includes
#include <stdlib.h>
#include <math.h>

// Inclusion guard
#ifndef __PERLINNOISE_H__
#define __PERLINNOISE_H__

class PerlinNoise
{
public:
    
    // Initialize with nothing
    PerlinNoise(int Width, int Height);
    
    // Release internals
    ~PerlinNoise();
    
    // Render a new map of the noise
    // Seed can be anything, but bigger numbers are better
    // Uses a seed to offset the cosine (interpolation) function
    // The zoom is how "dense" the information is (good value is 75)
    // while persistance is the frequency (good value is 0.5)
    // while octaves is about the consistency of the output
    void Render(double seed = 0.0, double zoom = 75.0, double persistance = 0.25, int octaves = 8);
    
    // Access generated buffer
    const unsigned char* GetBuffer();
    
    // Access a pixel-specific location
    unsigned char GetDepth(int x, int y);
    
    // Get the depth pre-computed as a float from [0, 1]
    float GetfDepth(int x, int y);
    
private:
    
    // Size of the noise map
    int Width, Height;
    
    // Noise output buffer
    unsigned char* NoiseBuffer;
    
    /*** Noise Generation ***/
    
    // Interpolate between two values based on a given factor
    double Interpolate(double a, double b, double x);
    
    // Get the local depth value
    double GetLocalNoise(double x, double y);
    
    // Get the global depth value considering  values
    double GetGlobalNoise(double x, double y);
    
};

// End inclusion guard
#endif
