/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "PerlinNoise.h"

#define PerlinNoise_setup(i,b0,b1,r0,r1)\
    t = vec[i] + 0x1000;\
    b0 = ((int)t) & (PerlinNoise_Size - 1);\
    b1 = (b0+1) & (PerlinNoise_Size - 1);\
    r0 = t - (int)t;\
    r1 = r0 - 1.0f;

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )
#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

PerlinNoise::PerlinNoise(int octaves, float freq, float amp, const char* Seed)
    : RandGen(Seed)
{
    // Save args
    mOctaves = octaves;
    mFrequency = freq;
    mAmplitude = amp;
    mSeed = RandGen.GetSeed();
    
    // Initialization
	int i, j, k;
    
	for (i = 0 ; i < PerlinNoise_Size ; i++)
    {
		p[i] = i;
		g1[i] = (float)((rand() % (PerlinNoise_Size + PerlinNoise_Size)) - PerlinNoise_Size) / PerlinNoise_Size;
		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (float)((rand() % (PerlinNoise_Size + PerlinNoise_Size)) - PerlinNoise_Size) / PerlinNoise_Size;
		normalize2(g2[i]);
		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (float)((rand() % (PerlinNoise_Size + PerlinNoise_Size)) - PerlinNoise_Size) / PerlinNoise_Size;
		normalize3(g3[i]);
	}
    
	while (--i)
    {
		k = p[i];
		p[i] = p[j = rand() % PerlinNoise_Size];
		p[j] = k;
	}
    
	for (i = 0 ; i < PerlinNoise_Size + 2 ; i++)
    {
		p[PerlinNoise_Size + i] = p[i];
		g1[PerlinNoise_Size + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[PerlinNoise_Size + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[PerlinNoise_Size + i][j] = g3[i][j];
	}
}

PerlinNoise::~PerlinNoise()
{
    // ...
}

float PerlinNoise::perlin_noise_2D(float vec[2])
{
    int terms    = mOctaves;
	float result = 0.0f;
    float amp = mAmplitude;
    
    vec[0]*=mFrequency;
    vec[1]*=mFrequency;
    
	for( int i=0; i<terms; i++ )
	{
		result += GetNoise2D(vec[0], vec[1])*amp;
		vec[0] *= 2.0f;
		vec[1] *= 2.0f;
        amp*=0.5f;
	}
    
    
	return result;
}

float PerlinNoise::GetNoise1D(float arg)
{
    int bx0, bx1;
	float rx0, rx1, sx, t, u, v, vec[1];
    
	vec[0] = arg;
    
	PerlinNoise_setup(0, bx0,bx1, rx0,rx1);
    
	sx = s_curve(rx0);
    
	u = rx0 * g1[ p[ bx0 ] ];
	v = rx1 * g1[ p[ bx1 ] ];
    
	return lerp(sx, u, v) + 0.5f;
}

float PerlinNoise::GetNoise2D(float x, float y)
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	int i, j;
    
    float vec[2] = {x, y};
	PerlinNoise_setup(0,bx0,bx1,rx0,rx1);
	PerlinNoise_setup(1,by0,by1,ry0,ry1);
    
	i = p[bx0];
	j = p[bx1];
    
	b00 = p[i + by0];
	b10 = p[j + by0];
	b01 = p[i + by1];
	b11 = p[j + by1];
    
	sx = s_curve(rx0);
	sy = s_curve(ry0);
    
	q = g2[b00];
	u = at2(rx0,ry0);
	q = g2[b10];
	v = at2(rx1,ry0);
	a = lerp(sx, u, v);
    
	q = g2[b01];
	u = at2(rx0,ry1);
	q = g2[b11];
	v = at2(rx1,ry1);
	b = lerp(sx, u, v);
    
	return lerp(sy, a, b) + 0.5f;
}

float PerlinNoise::GetNoise3D(float x, float y, float z)
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	int i, j;
    
    float vec[3] = {x, y, z};
	PerlinNoise_setup(0, bx0,bx1, rx0,rx1);
	PerlinNoise_setup(1, by0,by1, ry0,ry1);
	PerlinNoise_setup(2, bz0,bz1, rz0,rz1);
    
	i = p[ bx0 ];
	j = p[ bx1 ];
    
	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];
    
	t  = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);
    
	q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
	q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
	a = lerp(t, u, v);
    
	q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
	q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
	b = lerp(t, u, v);
    
	c = lerp(sy, a, b);
    
	q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
	q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
	a = lerp(t, u, v);
    
	q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
	q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
	b = lerp(t, u, v);
    
	d = lerp(sy, a, b);
    
	return lerp(sz, c, d) + 0.5f;
}

void PerlinNoise::normalize2(float v[2])
{
	float s;
    
	s = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
    s = 1.0f/s;
	v[0] = v[0] * s;
	v[1] = v[1] * s;
}

void PerlinNoise::normalize3(float v[3])
{
	float s;
    
	s = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    s = 1.0f/s;
    
	v[0] = v[0] * s;
	v[1] = v[1] * s;
	v[2] = v[2] * s;
}
