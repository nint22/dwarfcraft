/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "PlasmaNoise.h"

PlasmaNoise::PlasmaNoise(int Width, int Height)
{
    // Save and default values
    this->Width = Width;
    this->Height = Height;
    NoiseBuffer = new unsigned char[Width * Height];
}

PlasmaNoise::~PlasmaNoise()
{
    if(NoiseBuffer != NULL)
        delete[] NoiseBuffer;
}

void PlasmaNoise::Render(double seed, double zoom)
{
    // For each pixel
    for(int y = 0; y < Height; y++)
    for(int x = 0; x < Width; x++)
    {
        // Get pixel value and average
        double PixelNoise = (127.5 + 127.5 * sin((double)x / zoom + seed))
                          + (127.5 + 127.5 * sin((double)y / zoom + seed))
                          + (127.5 + 127.5 * sin(sqrt(float(x * x + y * y)) / zoom + seed));
        PixelNoise /= 3.0;
        
        // Set back down to 
        int PixelValue = (int)PixelNoise;
        if(PixelValue > 255)
            PixelValue = 255;
        else if(PixelValue < 0)
            PixelValue = 0;
        
        // Set the pixel value in memory
        NoiseBuffer[y * Width + x] = (unsigned char)PixelValue;
    }
}

const unsigned char* PlasmaNoise::GetBuffer()
{
    return NoiseBuffer;
}

unsigned char PlasmaNoise::GetDepth(int x, int y)
{
    // Bounds check the buffers
    // TODO - Need magi 3
    
    // Check buffer and/or return data
    if(NoiseBuffer == NULL)
        return 0;
    else
        return NoiseBuffer[y * Width + x];
}
