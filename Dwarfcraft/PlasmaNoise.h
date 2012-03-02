/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: PlasmaNoise.h/cpp
 Desc: Simple, clean, plasma - classic noise generation. Based
 on the "fast plasma": http://www.pouet.net/topic.php?which=4650
 
***************************************************************/

// Standard includes
#include "stdlib.h"
#include "math.h"

// Inclusion guard
#ifndef __PLASMANOISE_H__
#define __PLASMAOISE_H__

class PlasmaNoise
{
public:
    
    // Initialize with nothing
    PlasmaNoise(int Width, int Height);
    
    // Release internals
    ~PlasmaNoise();
    
    // Simple plasma render
    void Render(double seed = 0.0, double zoom = 32.0);
    
    // Access generated buffer
    const unsigned char* GetBuffer();
    
    // Accex a pixel-specific location
    unsigned char GetDepth(int x, int y);
    
private:
    
    // Size of the noise map
    int Width, Height;
    
    // Noise output buffer
    unsigned char* NoiseBuffer;
    
};

// End inclusion guard
#endif
