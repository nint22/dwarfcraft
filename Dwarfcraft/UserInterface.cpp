/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "UserInterface.h"

UserInterface::UserInterface(GrfxObject* Parent, Glui2* GluiHandle)
: GrfxObject(Parent)
{
    // Create the GUI (Dashboard must be made first, so it's below and hides the chains)
    DashboardController = new g2DashboardController(GluiHandle->GetRootController(), GluiHandle->GetMainTheme());
    ChatController = new g2ChatController(GluiHandle->GetRootController(), GluiHandle->GetMainTheme());
    TasksController = new g2TasksController(GluiHandle->GetRootController(), GluiHandle->GetMainTheme());
    
    // Create sliders
    DepthSlider = GluiHandle->AddSlider(0, 0);
    DepthSlider->SetVertical(true);
    DepthSlider->SetWidth(300);
    
    FovySlider = GluiHandle->AddSlider(0, 0);
    FovySlider->SetVertical(true);
    FovySlider->SetWidth(100);
    
    // Create the text labels
    int TabWidth;
    GluiHandle->GetMainTheme()->GetComponentSize("InfoTab", &TabWidth);
    
    DwarfCount = GluiHandle->AddLabel(20 + 0 * (TabWidth * 2 + 10), 8, "\x01 3 Dwarves");
    Position = GluiHandle->AddLabel(20 + 1 * (TabWidth * 2 + 10), 8, "Pos: 0, 0, 0");
    Depth = GluiHandle->AddLabel(20 + 2 * (TabWidth * 2 + 10), 8, "Depth: 0");
}

UserInterface::~UserInterface()
{
    // All Glui2 allocates are released by the Glui2 root..
}

g2ChatController* UserInterface::GetChatController()
{
    return ChatController;
}

g2TasksController* UserInterface::GetTasksController()
{
    return TasksController;
}

g2DashboardController* UserInterface::GetDashboardController()
{
    return DashboardController;
}

g2Slider* UserInterface::GetDepthSlider()
{
    return DepthSlider;
}

g2Slider* UserInterface::GetFovySlider()
{
    return FovySlider;
}

g2Label* UserInterface::GetPositionLabel()
{
    return Position;
}

g2Label* UserInterface::GetDepthLabel()
{
    return Depth;
}

void UserInterface::SetPos(int x, int y, int z)
{
    // Format buffer
    char PosBuffer[64];
    sprintf(PosBuffer, "Pos: %d, %d, %d", x, y, z);
    Position->SetText(PosBuffer);
}

void UserInterface::SetDepth(int LayerCutoff)
{
    // Format buffer
    char PosBuffer[64];
    sprintf(PosBuffer, "Depth: %d", LayerCutoff);
    Depth->SetText(PosBuffer);
}

void UserInterface::Update(float dT)
{
    // Nothing to do...
}

void UserInterface::WindowResizeEvent(int NewWidth, int NewHeight)
{
    // Upate sliders (top-right)
    DepthSlider->SetPos(NewWidth - 20, 28);
    FovySlider->SetPos(NewWidth - 36, 28);
    
    // Bottom-left pos
    ChatController->SetPos(0, NewHeight - ChatController->GetHeight());
    
    // Bottom-right pos
    TasksController->SetPos(NewWidth - TasksController->GetWidth(), NewHeight - TasksController->GetHeight());
    
    // Center
    DashboardController->SetPos(NewWidth / 2 - DashboardController->GetWidth() / 2, NewHeight - DashboardController->GetHeight());
}
