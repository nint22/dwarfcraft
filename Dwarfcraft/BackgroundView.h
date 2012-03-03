/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: BackgroundView.h/cpp
 Desc: Manages the background of the world, moving the sun 
 and moon, as well as dealing with weather events.
 
 The time system in this game is exactly like the Minecraft
 time mechanisms. Ranges from 0 and 24000 of "Minecraft seconds"
 
 1 Minecraft Second ≈ 0.013 seconds of gameplay.
 1 Minecraft Minute ≈ 0.8 seconds of gameplay.
 1 Minecraft Hour = 50 seconds of gameplay.
 1 Minecraft Day = 20 minutes of gameplay.
 
 The "TotalTime" variable is measured in Minecraft seconds, thus:
 Daytime is 
 
 1. 0 - 3000 dawn
 2. 3000 - 15000 midday // 1200 is high-noon
 3. 15000 - 18000 dusk
 4. 18000 - 24000 night // 2400 is high-moon
 
***************************************************************/

#ifndef __BACKGROUNDVIEW_H__
#define __BACKGROUNDVIEW_H__

#include <Glui2/Glui2.h>
#include "GrfxObject.h"
#include "Vector3.h"

// A helpful index of time ranges of each event
static const float BackgroundView_TimeRanges[4][2] = {
    {0, 3000},      // Morning
    {3000, 15000},  // Day
    {15000, 18000}, // Evening
    {18000, 24000}, // Night
};

// The colors associated with the above times
static const Vector3<float> BackgroundView_TimeColors[4] = 
{
    Vector3<float>(163.0f / 255.0f, 63.0f / 255.0f, 53.0f / 255.0f),
    Vector3<float>(142.0f / 255.0f, 177.0f / 255.0f, 254.0f / 255.0f),
    Vector3<float>(42.0f / 255.0f, 41.0f / 255.0f, 55.0f / 255.0f),
    Vector3<float>(5.0f / 255.0f, 5.0f / 255.0f, 20.0f / 255.0f),
};

class BackgroundView : public GrfxObject
{
public:
    
    // Standard constructor and desctructor
    BackgroundView(GrfxObject* Parent, int ObjectDepth);
    ~BackgroundView();
    
    // Pass the current camera angle, so we only draw the sun / moon in the same area..
    void SetCameraAngle(float CameraAngle, float CameraPitch);
    
protected:
    
    // Standard render and update functions from GrfxObject
    void Render();
    void Update(float dT);
    void WindowResizeEvent(int NewWidth, int NewHeight);
    
private:
    
    // Render a square on-screen using the 2D ortho system
    void RenderScreenTexture(int x, int y, int width, int height, GLuint TextureID);
    
    // Color pair based on the current time
    void GetSkyColorPair(float Time, Vector3<float>* BottomColor, Vector3<float>* TopColor);
    
    // Total elapsed time
    float TotalTime;
    
    // Current window width / height
    int WindowWidth, WindowHeight;
    
    // Used to translate the sun and moon a little
    float CameraAngle, CameraPitch;
    
    // Moon and sun texture handles
    GLuint SunTextureID, MoonTextureID;
};

#endif
