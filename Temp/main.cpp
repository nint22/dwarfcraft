#include "window.h"
#include "perlin.h"
#include <mmsystem.h>
#include <math.h>

// Rotation matrix
float Mtx[9];

// Our bitmap
static DWORD map[256*256];

// Create the bitmap info header used to render the bitmap
static BITMAPINFOHEADER bmih = 
{
	sizeof(BITMAPINFOHEADER),
	256,256,1,32,BI_RGB,0,0,0,0,0
};

void RenderNoise(HWND hWnd, CPerlin *pPerlin, float z = 0);
void InitRotationMatrix(const float *pAxis, float r);

//=============================================================================

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, 
				   LPSTR pCmdLine, int showCmd)
{
	// Initialize window
	CWindow *pWindow = new CWindow();
	CWindow::SParameters params;
	pWindow->SetDefault(&params);
	params.width = 256;
	params.height = 256;
	params.pTitle = "Perlin Noise Test";
	params.hInstance = hInst;

	if( FAILED(pWindow->Initialize(params)) )
		return 0;	

	// Initialize perlin noise
	CPerlin *pPerlin = new CPerlin();
	pPerlin->Initialize(timeGetTime());

	// Initialize rotation matrix with random values
	// This will make sure our bitmap will not be  
	// parallell to the planes of the noise function
	float Axis[3] = {0.16f, 0.67f, 0.43f};
	InitRotationMatrix(Axis, 0.34521f);

	while( !pWindow->CheckMessage(false) )
	{
		// Render the perlin noise
		RenderNoise(pWindow->GetHandle(), pPerlin, timeGetTime()/4000.0f);
	}

	delete pWindow;

	return 0;
}

//=============================================================================

void RenderNoise(HWND hWnd, CPerlin *pPerlin, float z)
{
	// Create a DC for the client area
	HDC dc = GetDC(hWnd);

	// Fill the bitmap with the noise
	for( int iy = 0; iy < 256; iy++ )
	{
		// Compute the starting position from the y and z coordinate
		float y = iy/16.0f;
		float p[3] = { y*Mtx[1] + z*Mtx[2], 
			           y*Mtx[4] + z*Mtx[5], 
					   y*Mtx[7] + z*Mtx[8] };

		// This represents movements along the x axis
		float x = 1/16.0f;
		float d[3] = { x*Mtx[0], x*Mtx[3], x*Mtx[6] };

		for( int ix = 0; ix < 256; ix++ )
		{
			BYTE n = BYTE(255*0.5f*(pPerlin->Noise3(p[0], p[1], p[2]) + 1));

			map[ix+iy*256] = (n<<16) | (n<<8) | n;

			p[0] += d[0];
			p[1] += d[1];
			p[2] += d[2];
		}
	}

	// Render the bitmap to the DC
	StretchDIBits(dc, 0, 0, 256, 256, 0, 0, 256, 256,  
		          map, (BITMAPINFO*)&bmih, DIB_RGB_COLORS, SRCCOPY);
}

//=============================================================================

void InitRotationMatrix(const float *pAxis, float r)
{
  // The axis vector must be of unit length
  float x, y, z, m;
  m = sqrtf(pAxis[0]*pAxis[0] + pAxis[1]*pAxis[1] + pAxis[2]*pAxis[2]);
  x = pAxis[0]/m;
  y = pAxis[1]/m;
  z = pAxis[2]/m;

  // Compute the rotation matrix  
  float c = (float)cos(r);
  float s = (float)sin(r);

  Mtx[0] = (x * x) * (1.0f - c) + c;
  Mtx[1] = (y * x) * (1.0f - c) + (z * s);
  Mtx[2] = (z * x) * (1.0f - c) - (y * s);

  Mtx[3] = (x * y) * (1.0f - c) - (z * s);
  Mtx[4] = (y * y) * (1.0f - c) + c;
  Mtx[5] = (z * y) * (1.0f - c) + (x * s);

  Mtx[6] = (x * z) * (1.0f - c) + (y * s);
  Mtx[7] = (y * z) * (1.0f - c) - (x * s);
  Mtx[8] = (z * z) * (1.0f - c) + c;
} 

