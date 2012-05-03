/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "PerlinNoise.h"

PerlinNoise::PerlinNoise(const char* Seed)
    : RandGen(Seed)
{
	// Initialize the permutation table with a linear seed
	for(int i = 0; i < PerlinNoise_Size; i++)
		p[i] = i;
    
    // Randomize data set
	for(int i = 0; i < PerlinNoise_Size; i++)
	{
        // Choose swap
		int j = RandGen.Rand() % PerlinNoise_Size;
        
        // Swap
		int nSwap = p[i];
		p[i] = p[j];
		p[j] = nSwap;
	}
    
	// Generate the gradient lookup tables
	for(int i = 0; i < PerlinNoise_Size; i++)
	{
        // Note from original author:
		// Ken Perlin proposes that the gradients are taken from the unit 
		// circle/sphere for 2D/3D, but there are no noticable difference 
		// between that and what I'm doing here. For the sake of generality 
		// I will not do that.
		gx[i] = float(RandGen.Rand())/(UINT_MAX/2) - 1.0f; 
		gy[i] = float(RandGen.Rand())/(UINT_MAX/2) - 1.0f;
		gz[i] = float(RandGen.Rand())/(UINT_MAX/2) - 1.0f;
	}
}

PerlinNoise::~PerlinNoise()
{
    
}


float PerlinNoise::GetNoise1D(float x)
{
	// Compute what gradients to use
	int qx0 = (int)floorf(x);
	int qx1 = qx0 + 1;
	float tx0 = x - (float)qx0;
	float tx1 = tx0 - 1;
    
	// Make sure we don't come outside the lookup table
	qx0 = qx0 % PerlinNoise_Size;
	qx1 = qx1 % PerlinNoise_Size;
    
	// Compute the dotproduct between the vectors and the gradients
	float v0 = gx[qx0]*tx0;
	float v1 = gx[qx1]*tx1;
    
	// Modulate with the weight function
	float wx = (3 - 2*tx0)*tx0*tx0;
	float v = v0 - wx*(v0 - v1);
    
	return v + 0.5f;
}

float PerlinNoise::GetNoise2D(float x, float y)
{
	// Compute what gradients to use
	int qx0 = (int)floorf(x);
	int qx1 = qx0 + 1;
	float tx0 = x - (float)qx0;
	float tx1 = tx0 - 1;
    
	int qy0 = (int)floorf(y);
	int qy1 = qy0 + 1;
	float ty0 = y - (float)qy0;
	float ty1 = ty0 - 1;
    
	// Make sure we don't come outside the lookup table
	qx0 = qx0 % PerlinNoise_Size;
	qx1 = qx1 % PerlinNoise_Size;
    
	qy0 = qy0 % PerlinNoise_Size;
	qy1 = qy1 % PerlinNoise_Size;
    
	// Permutate values to get pseudo randomly chosen gradients
	int q00 = p[(qy0 + p[qx0]) % PerlinNoise_Size];
	int q01 = p[(qy0 + p[qx1]) % PerlinNoise_Size];
    
	int q10 = p[(qy1 + p[qx0]) % PerlinNoise_Size];
	int q11 = p[(qy1 + p[qx1]) % PerlinNoise_Size];
    
	// Compute the dotproduct between the vectors and the gradients
	float v00 = gx[q00]*tx0 + gy[q00]*ty0;
	float v01 = gx[q01]*tx1 + gy[q01]*ty0;
    
	float v10 = gx[q10]*tx0 + gy[q10]*ty1;
	float v11 = gx[q11]*tx1 + gy[q11]*ty1;
    
	// Modulate with the weight function
	float wx = (3 - 2*tx0)*tx0*tx0;
	float v0 = v00 - wx*(v00 - v01);
	float v1 = v10 - wx*(v10 - v11);
    
	float wy = (3 - 2*ty0)*ty0*ty0;
	float v = v0 - wy*(v0 - v1);
    
	return v + 0.5f;
}

float PerlinNoise::GetNoise3D(float x, float y, float z)
{
	// Compute what gradients to use
	int qx0 = (int)floorf(x);
	int qx1 = qx0 + 1;
	float tx0 = x - (float)qx0;
	float tx1 = tx0 - 1;
    
	int qy0 = (int)floorf(y);
	int qy1 = qy0 + 1;
	float ty0 = y - (float)qy0;
	float ty1 = ty0 - 1;
    
	int qz0 = (int)floorf(z);
	int qz1 = qz0 + 1;
	float tz0 = z - (float)qz0;
	float tz1 = tz0 - 1;
    
	// Make sure we don't come outside the lookup table
	qx0 = qx0 % PerlinNoise_Size;
	qx1 = qx1 % PerlinNoise_Size;
    
	qy0 = qy0 % PerlinNoise_Size;
	qy1 = qy1 % PerlinNoise_Size;
    
	qz0 = qz0 % PerlinNoise_Size;
	qz1 = qz1 % PerlinNoise_Size;
    
	// Permutate values to get pseudo randomly chosen gradients
	int q000 = p[(qz0 + p[(qy0 + p[qx0]) % PerlinNoise_Size]) % PerlinNoise_Size];
	int q001 = p[(qz0 + p[(qy0 + p[qx1]) % PerlinNoise_Size]) % PerlinNoise_Size];
    
	int q010 = p[(qz0 + p[(qy1 + p[qx0]) % PerlinNoise_Size]) % PerlinNoise_Size];
	int q011 = p[(qz0 + p[(qy1 + p[qx1]) % PerlinNoise_Size]) % PerlinNoise_Size];
    
	int q100 = p[(qz1 + p[(qy0 + p[qx0]) % PerlinNoise_Size]) % PerlinNoise_Size];
	int q101 = p[(qz1 + p[(qy0 + p[qx1]) % PerlinNoise_Size]) % PerlinNoise_Size];
    
	int q110 = p[(qz1 + p[(qy1 + p[qx0]) % PerlinNoise_Size]) % PerlinNoise_Size];
	int q111 = p[(qz1 + p[(qy1 + p[qx1]) % PerlinNoise_Size]) % PerlinNoise_Size];
    
	// Compute the dotproduct between the vectors and the gradients
	float v000 = gx[q000]*tx0 + gy[q000]*ty0 + gz[q000]*tz0;
	float v001 = gx[q001]*tx1 + gy[q001]*ty0 + gz[q001]*tz0;  
    
	float v010 = gx[q010]*tx0 + gy[q010]*ty1 + gz[q010]*tz0;
	float v011 = gx[q011]*tx1 + gy[q011]*ty1 + gz[q011]*tz0;
    
	float v100 = gx[q100]*tx0 + gy[q100]*ty0 + gz[q100]*tz1;
	float v101 = gx[q101]*tx1 + gy[q101]*ty0 + gz[q101]*tz1;  
    
	float v110 = gx[q110]*tx0 + gy[q110]*ty1 + gz[q110]*tz1;
	float v111 = gx[q111]*tx1 + gy[q111]*ty1 + gz[q111]*tz1;
    
	// Modulate with the weight function
	float wx = (3 - 2*tx0)*tx0*tx0;
	float v00 = v000 - wx*(v000 - v001);
	float v01 = v010 - wx*(v010 - v011);
	float v10 = v100 - wx*(v100 - v101);
	float v11 = v110 - wx*(v110 - v111);
    
	float wy = (3 - 2*ty0)*ty0*ty0;
	float v0 = v00 - wy*(v00 - v01);
	float v1 = v10 - wy*(v10 - v11);
    
	float wz = (3 - 2*tz0)*tz0*tz0;
	float v = v0 - wz*(v0 - v1);
    
	return v + 0.5f;
}
