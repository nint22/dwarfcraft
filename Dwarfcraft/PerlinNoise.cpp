/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "PerlinNoise.h"

PerlinNoise::PerlinNoise(int Width, int Height)
{
    // Save and default values
    this->Width = Width;
    this->Height = Height;
    NoiseBuffer = new unsigned char[Width * Height];
}

PerlinNoise::~PerlinNoise()
{
    if(NoiseBuffer != NULL)
        delete[] NoiseBuffer;
}

void PerlinNoise::Render(double seed, double zoom, double persistance, int octaves)
{
    // Grow seed just to help
    seed *= 10000.0;
    
    // For each pixel
    for(int y = 0; y < Height; y++)
    for(int x = 0; x < Width; x++)
    {
        // Summation of noise
        double PixelNoise = 0.0;
        
        // For each octave -1
        for(int i = 0; i < octaves - 1; i++)
        {
            // Compute the next octave pattern
            double frequency = pow(2.0, i);
            double amplitude = pow(persistance, i);
            
            // Does not always returns -1 to 1 (we normalize the trig functions)
            PixelNoise += GetGlobalNoise( double(x + seed) * frequency / zoom, double(y + seed) / zoom * frequency) * amplitude;
        }
        
        // Set back down to 
        int PixelValue = (int)((PixelNoise * 128.0) + 128.0);
        if(PixelValue > 255)
            PixelValue = 255;
        else if(PixelValue < 0)
            PixelValue = 0;
        
        // Set the pixel value in memory
        NoiseBuffer[y * Width + x] = (unsigned char)PixelValue;
    }
}

const unsigned char* PerlinNoise::GetBuffer()
{
    return NoiseBuffer;
}

unsigned char PerlinNoise::GetDepth(int x, int y)
{
    // Bounds check the buffers
    // TODO - Need magi 3
    
    // Check buffer and/or return data
    if(NoiseBuffer == NULL)
        return 0;
    else
        return NoiseBuffer[y * Width + x];
}

float PerlinNoise::GetfDepth(int x, int y)
{
    return float(GetDepth(x, y)) / 255.0f;
}

double PerlinNoise::Interpolate(double a, double b, double x)
{
    // Trig-interpolation
    double ft = x * 3.1415927;
    double f = (1.0 - cos(ft)) / 2.0f;
    return a * (1.0 - f) + (b * f);
}

double PerlinNoise::GetLocalNoise(double x, double y)
{
    // Pseudo-random noise generation (using hard-coded approach)
    int n=(int)x+(int)y*57;
    n=(n<<13)^n; // Not power of, but bit-wise NOT
    int nn=(n*(n*n*60493+19990303)+1376312589) & 0x7fffffff;
    
    return 1.0 - ((double)nn/1073741824.0);
}

double PerlinNoise::GetGlobalNoise(double x, double y)
{
    // Floor and compute locale noise
    double floorx = (int)x;
    double floory = (int)y;
    double s, t, u, v;
    
    // Surounding localized pixels
    s=GetLocalNoise(floorx, floory); 
    t=GetLocalNoise(floorx+1 ,floory);
    u=GetLocalNoise(floorx, floory+1);
    v=GetLocalNoise(floorx+1, floory+1);
    
    // Interpolate between the vertical and horizonal (i.e. 1D)
    double int1 = Interpolate(s, t, x - floorx);
    double int2 = Interpolate(u, v, x - floorx);
    
    // Interpoate between both (i.e. 2D)
    return Interpolate(int1, int2, y - floory);
}
