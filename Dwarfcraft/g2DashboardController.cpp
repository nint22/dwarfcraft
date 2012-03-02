/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "g2DashboardController.h"

g2DashboardController::g2DashboardController(g2Controller* Owner, g2Theme* MainTheme)
:g2Controller(Owner, MainTheme)
{
    // Set size
    int CompWidth, CompHeight;
    GetTheme()->GetComponentSize("DashboardController", &CompWidth, &CompHeight);
    SetSize(CompWidth * 4, CompHeight * 4);
    
    // Allocate the label's text
    ClockText = new g2Label(this, GetTheme());
    ClockText->SetText("Time: 23:02, Season: Winter");
}

g2DashboardController::~g2DashboardController()
{
    
}

int g2DashboardController::GetWidth()
{
    return Width;
}

int g2DashboardController::GetHeight()
{
    return Height;
}

void g2DashboardController::SetSize(int Width, int Height)
{
    // Save size
    this->Width = Width;
    this->Height = Height;
}

void g2DashboardController::SetTime(int Season, int Day, float Time)
{
    // Render string
    // Note: Should blink in the future
    char LabelBuffer[256];
    sprintf(LabelBuffer, "Season: %d, Day: %02d, Time %d:%02d", Season, Day, int(Time), int((Time - int(Time)) * 100.0f));
    ClockText->SetText(LabelBuffer);
    
    // Update position
    ClockText->SetPos(Width / 2 - ClockText->GetWidth() / 2, GetTheme()->GetCharacterHeight());
}

void g2DashboardController::Render(int x, int y)
{
    /** Bottom **/
    
    // Chain sizes
    int ChainWidth, ChainHeight;
    GetTheme()->GetComponentSize("Chains", &ChainWidth, &ChainHeight);
    
    // Draw the chains first (two on each side, going away from the objects center)
    ChainWidth *= 2;
    ChainHeight *= 2;
    
    DrawComponent("Chains", x - ChainWidth, y + ChainHeight, ChainWidth, ChainHeight);
    DrawComponent("Chains", x + Width, y + ChainHeight, ChainWidth, ChainHeight);
    
    DrawComponent("Chains", x - ChainWidth, y + Height - ChainHeight, ChainWidth, ChainHeight);
    DrawComponent("Chains", x + Width, y + Height - ChainHeight, ChainWidth, ChainHeight);
    
    // Render main self
    DrawComponent("DashboardController", x, y, Width, Height);
    
    /** Top **/
    
    // Draw as many chains as needed
    int MaxChains = ceilf(float(WindowWidth) / float(ChainWidth));
    for(int i = 0; i < MaxChains; i++)
        DrawComponent("Chains", i * ChainWidth, -4, ChainWidth, ChainHeight);
    
    // Draw the sliders for fovy, etc.
    // They self-draw, we just need to explicitly draw the icons
    // Depth: - 20, 28
    // Fovy: - 36, 28
    DrawComponent("DepthIcon", WindowWidth - 20, 18);
    DrawComponent("ZoomIcon", WindowWidth - 36, 18);
    
    // Draw some of the status / information elements
    int TabWidth, TabHeight;
    GetTheme()->GetComponentSize("InfoTab", &TabWidth, &TabHeight);
    
    for(int i = 0; i < 3; i++)
        DrawComponent("Infotab", 10 + i * (TabWidth * 2 + 10), 4, TabWidth * 2, TabHeight * 2);
}

void g2DashboardController::Update(float dT)
{
    // ...
}

void g2DashboardController::WindowResizeEvent(int NewWidth, int NewHeight)
{
    // Save for top bar
    WindowWidth = NewWidth;
}
