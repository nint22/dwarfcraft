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

// Included for access to the designations type and names
#include "DesignationsView.h"

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
    // If any of the elements are negative, ignore and just reposition as needed
    void SetTime(int Season = -1, int Day = -1, float Time = -1);
    
    // Set the output size
    void SetSize(int Width, int Height);
    
    // Are we in selection mode?
    bool IsSelecting();
    
    // Update the selection text
    void SetSelectionVolume(Vector3<int> SelectStart, Vector3<int> SelectEnd, bool IsDone);
    
protected:
    
    // Render the controller
    void Render(int x, int y);
    
    // Render the bottom controller icons
    void Render(int x, int y, DesignationType* Types, int TypeCount);
    
    // Update the controller
    void Update(float dT);
    
    // Catch any window resize event
    void WindowResizeEvent(int NewWidth, int NewHeight);
    
    // When the mouse hovers over any of the buttons, make a selection
    void MouseHover(int x, int y);
    
    // Mouse click event
    void MouseClick(g2MouseButton button, g2MouseClick state, int x, int y);
    
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
    
    // Custom text that is only manually rendered
    g2Label* CustomText;
    
    // Current mouse position
    int MouseX, MouseY;
    
    /*** Current Designations ***/
    
    // Current designation groups:
    // 0: Main, 1: Construction, 2: storage, 3: Collections, 4: Military
    int DesignationGroup;
    
    // Allocated list of designations
    DesignationType Types[DesignationCount];
    int TypeCount;
    
    // True if selecting a volume
    bool Selecting;
    DesignationType SelectingType;
    
    /*** Icon Constants ***/
    
    // Icon and button sizes
    int IconW, IconH;
    int ButtonW, ButtonH;
    
    // Offset from the origin of the controller
    int IconOffsetX, IconOffsetY;
    
    // Difference between icon and button volumes
    int dx, dy;
    
    // Space between each icon
    int IconSpaceY, IconSpaceX;
    
    // Total number of icons drawn in a row
    int IconColCount;
};

#endif
