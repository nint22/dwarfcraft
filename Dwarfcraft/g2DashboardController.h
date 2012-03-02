/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: g2DashboardController.h/cpp
 Desc: A custom g2Controller; implements the on-screen clock,
 based on a texture that moves the sun / moon over time, and all
 the other centralized commands. It also owns the side and top bars.
 
***************************************************************/

// Inclusion guard
#ifndef __G2DASHBOARDCONTROLLER_H__
#define __G2DASHBOARDCONTROLLER_H__

#include <Glui2/glui2.h>

static const int g2DashboardController_SideWidth = 4;

class g2DashboardController : public g2Controller
{
public:
    
    // Standard constructor / destructor
    g2DashboardController(g2Controller* Owner, g2Theme* MainTheme);
    ~g2DashboardController();
    
    // Set target width
    void SetWidth(int Width);
    
    // Get width
    int GetWidth();
    
    // Get height
    int GetHeight();
    
    // Set the user interface's time; day, season, time (time is based on a 24-hour, 60 minute fraction)
    void SetTime(int Season, int Day, float Time);
    
    // Set the output size
    void SetSize(int Width, int Height);
    
protected:
    
    // Render the controller
    void Render(int x, int y);
    
    // Update the controller
    void Update(float dT);
    
    // Catch any window resize event
    void WindowResizeEvent(int NewWidth, int NewHeight);
    
private:
    
    // Target width
    int Width, Height;
    
    // Current screen size
    int WindowWidth;
    
    // Bottom text
    g2Label* ClockText;
    
    // Depth and zoom sliders
    g2Slider* DepthSlider;
    g2Slider* FovySlider;
};

#endif
