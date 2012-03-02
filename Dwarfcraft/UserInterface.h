/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: UserInterface.h/cpp
 Desc: The root User Interface (not a g2Controller itself) that
 manages all on-screen controller elements. This class, and not
 the sub-controllers, are used to post updates from the game to
 the user, as well as vice-versa.
 
***************************************************************/

// Inclusion guard
#ifndef __USERINTERFACE_H__
#define __USERINTERFACE_H__

#include <Glui2/glui2.h>
#include "GrfxObject.h"
#include "g2DashboardController.h"
#include "g2TasksController.h"
#include "g2ChatController.h"
#include "g2EntityController.h"
#include "WorldView.h"
#include "DesignationsView.h"

class UserInterface : public GrfxObject
{
public:
    
    // Constructor and destructor
    // Automatically builds the scene
    UserInterface(GrfxObject* Parent, Glui2* GluiHandle);
    ~UserInterface();
    
    // Access each of the three main controllers
    g2ChatController* GetChatController();
    g2TasksController* GetTasksController();
    g2DashboardController* GetDashboardController();
    
    // Get the depth slider and fovy slider
    g2Slider* GetDepthSlider();
    g2Slider* GetFovySlider();
    
    // Get the three top-left labels
    g2Label* GetDwarfCountLabel();
    g2Label* GetPositionLabel();
    g2Label* GetDepthLabel();
    
    // Set camera pos
    void SetPos(int x, int y, int z);
    
    // Set depth
    void SetDepth(int LayerCutoff);
    
protected:
    
    // Update used for parsing input events
    void Update(float dT);
    
    // Update controllers based on window size change
    void WindowResizeEvent(int NewWidth, int NewHeight);
    
private:
    
    /*** GUI Elements ***/
    
    g2Slider* DepthSlider;
    g2Slider* FovySlider;
    
    g2Label* DwarfCount;
    g2Label* Position;
    g2Label* Depth;
    
    /*** GUI Controllers ***/
    
    // Left
    g2ChatController* ChatController;
    
    // Right
    g2TasksController* TasksController;
    
    // Middle
    g2DashboardController* DashboardController;
};

// End of inclusion guard
#endif
