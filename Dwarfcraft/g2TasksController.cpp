/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "g2TasksController.h"

g2TasksController::g2TasksController(g2Controller* Owner, g2Theme* MainTheme)
: g2Controller(Owner, MainTheme)
{
    // Set size
    int CompWidth, CompHeight;
    GetTheme()->GetComponentSize("TasksController", &CompWidth, &CompHeight);
    SetSize(CompWidth * 4, CompHeight * 4);
}

g2TasksController::~g2TasksController()
{
    
}

void g2TasksController::SetSize(int Width, int Height)
{
    this->Width = Width;
    this->Height = Height;
}

int g2TasksController::GetWidth()
{
    return Width;
}

int g2TasksController::GetHeight()
{
    return Height;
}

void g2TasksController::Render(int x, int y)
{
    // Render a bar graph
    DrawComponent("TasksController", x, y, Width, Height);
    
    // Draw the three bars (R,G,B)
}
