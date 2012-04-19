/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "g2DashboardController.h"

g2DashboardController::g2DashboardController(g2Controller* Owner, g2Theme* MainTheme)
    : g2Controller(Owner, MainTheme)
{
    // Set size
    int CompWidth, CompHeight;
    GetTheme()->GetComponentSize("DashboardController", &CompWidth, &CompHeight);
    SetSize(CompWidth * 4, CompHeight * 4);
    
    // Allocate the label's text
    ClockText = new g2Label(this, GetTheme());
    
    // Allocate a custom text renderer without management
    CustomText = new g2Label(NULL, MainTheme);
    
    // Get the initial size of the icon (Armory is just one of any regular icon)
    GetTheme()->GetComponent("IconsList", &IconSrcW, &IconSrcH, &IconSrcX, &IconSrcY);
    GetTheme()->GetComponentSize("IconsList", &IconW, &IconH);
    GetTheme()->GetComponentSize("IconButton", &ButtonW, &ButtonH);
    
    // Double all icon sizes
    IconW *= 2;
    IconH *= 2;
    ButtonW *= 2;
    ButtonH *= 2;
    
    // Top and left spacing buffers
    IconOffsetX = 15;
    IconOffsetY = 45;
    dx = ButtonW / 2 - IconW / 2;
    dy = ButtonH / 2 - IconH / 2;
    IconSpaceY = 45;
    IconSpaceX = 45;
    
    // How many icons we show on a level
    IconColCount = 4;
    
    // No initial list or group type
    State = DashboardState_Root;
    
    // Not currently selecting
    Selecting = false;
    TypeCount = 0;
}

g2DashboardController::~g2DashboardController()
{
    
}

void g2DashboardController::GetIconInfo(IconType Type, float* SrcX, float* SrcY, float* SrcW, float* SrcH)
{
    // Post the size and grid offset
    *SrcW = IconSrcW;
    *SrcH = IconSrcH;
    *SrcX = float((int)Type % 8) * IconSrcW;
    *SrcY = float((int)Type / 8) * IconSrcH;
}

void g2DashboardController::SetDesignationsList(VolumeView* Designations)
{
    this->Designations = Designations;
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

bool g2DashboardController::IsSelecting()
{
    return Selecting;
}

void g2DashboardController::SetSelectionVolume(Vector3<int> SelectStart, Vector3<int> SelectEnd)
{
    // Save the points
    this->SelectStart = Vector3<int>((SelectStart.x <= SelectEnd.x) ? SelectStart.x : SelectEnd.x,
                               (SelectStart.y <= SelectEnd.y) ? SelectStart.y : SelectEnd.y,
                               (SelectStart.z <= SelectEnd.z) ? SelectStart.z : SelectEnd.z);
    this->SelectEnd = Vector3<int>((SelectStart.x > SelectEnd.x) ? SelectStart.x : SelectEnd.x,
                             (SelectStart.y > SelectEnd.y) ? SelectStart.y : SelectEnd.y,
                             (SelectStart.z > SelectEnd.z) ? SelectStart.z : SelectEnd.z);
}

void g2DashboardController::SetTime(int Season, int Day, float Time)
{
    // Render string
    // Only update if all ements are positive
    if(Season > 0 && Day > 0 && Time > 0)
    {
        char LabelBuffer[256];
        sprintf(LabelBuffer, "%d:%02d, %d, Winter", (int)Time, 0, Day);
        ClockText->SetText(LabelBuffer);
    }
    
    // Update position
    ClockText->SetPos(Width / 2 - ClockText->GetWidth() / 2, GetTheme()->GetCharacterHeight() + 4);
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
    
    /** Designations **/
    
    // Render default 4 categories
    if(State == DashboardState_Root)
    {
        IconType Types[4] = {Icon_RootMenu_Build, Icon_RootMenu_Designations, Icon_RootMenu_Stockpiles, Icon_RootMenu_Zones};
        Render(x, y, Types, 4);
    }
    // In a group
    else if(State == DashboardState_Group)
    {
        if(StateGroup == UI_RootMenu_Build)
        {
            IconType Types[5] = {Icon_BuildMenu_Farm, Icon_BuildMenu_Architecture, Icon_BuildMenu_Workshops, Icon_BuildMenu_Furniture, Icon_BuildMenu_Mechanical};
            Render(x, y, Types, 5);
        }
        else if(StateGroup == UI_RootMenu_Designations)
        {
            IconType Types[5] = {Icon_DesignationMenu_Fell, Icon_DesignationMenu_Forage, Icon_DesignationMenu_Mine, Icon_DesignationMenu_Fill, Icon_DesignationMenu_Flood};
            Render(x, y, Types, 5);
        }
        else if(StateGroup == UI_RootMenu_Stockpiles)
        {
            IconType Types[8] = {Icon_StockpilesMenu_Rubbish, Icon_StockpilesMenu_Food, Icon_StockpilesMenu_Crafted, Icon_StockpilesMenu_Equipment, Icon_StockpilesMenu_RawResources, Icon_StockpilesMenu_Ingots, Icon_StockpilesMenu_Graves, Icon_StockpilesMenu_Wood};
            Render(x, y, Types, 8);
        }
        else if(StateGroup == UI_RootMenu_Zones)
        {
            IconType Types[3] = {Icon_ZonesMenu_Hall, Icon_ZonesMenu_Pen, Icon_ZonesMenu_Defend};
            Render(x, y, Types, 3);
        }
    }
    // Actively selecting, only show a commit and reject button
    else if(State == DashboardState_Selection)
    {
        // We can cast directly from a DesignationType to an IconType, they are the same (initial) offsets
        IconType Types[2] = {Icon_Accept, Icon_Cancel};
        Render(x, y, Types, 2);
    }
    // Else, no such group
    else
        UtilAbort("This sub-menu does not exist");
}

void g2DashboardController::Render(int x, int y, IconType* Types, int TypeCount)
{
    // Copy the designations
    for(int i = 0; i < TypeCount; i++)
        this->Types[i] = Types[i];
    this->TypeCount = TypeCount;
    
    // For each icon
    for(int i = 0; i < TypeCount; i++)
    {
        // Positions for icon background, top-icon needs to do + <dx, dy>
        int ix = x + IconOffsetX + IconSpaceX * (i % IconColCount);
        int iy = y + IconOffsetY + IconSpaceY * (i / IconColCount);
        
        // Background image
        DrawComponent("IconButton", ix, iy, ButtonW, ButtonH);
        
        // Draw icon
        float sx, sy, sw, sh;
        GetIconInfo(Types[i], &sx, &sy, &sw, &sh);
        DrawComponent(ix + dx, iy + dy, IconW, IconH, sx, sy, sw, sh);
        
        // If the mouse is in this position, act as though the user is hovering over it
        // and draw a label of the text as well
        if(MouseX >= ix - x && MouseX < ix - x + ButtonW && MouseY >= iy - y && MouseY < iy - y + ButtonH)
        {
            // Only draw the hover if not the designation type when in selection mode
            if(State != DashboardState_Selection || i != 0)
                DrawComponent("IconButton_Hover", ix, iy, ButtonW, ButtonH);
            
            // Cancel button if non-root and last element
            if(State != DashboardState_Root && i == TypeCount - 1)
                ClockText->SetText("Cancel");
            
            // Commit button if selection and second-last element
            else if(State == DashboardState_Selection && i == 1)
                ClockText->SetText("Accept");
            
            // Root view
            else if(State == DashboardState_Root)
                ClockText->SetText(UI_RootMenuNames[i]);
            
            // All other views are just direct icon names
            else
            {
                if(StateGroup == UI_RootMenu_Build)
                    ClockText->SetText(UI_BuildMenuNames[Types[i]]);
                else if(StateGroup == UI_RootMenu_Designations)
                    ClockText->SetText(UI_DesignationsMenuNames[Types[i]]);
                else if(StateGroup == UI_RootMenu_Stockpiles)
                    ClockText->SetText(UI_StockpilesMenuNames[Types[i]]);
                else if(StateGroup == UI_RootMenu_Zones)
                    ClockText->SetText(UI_ZonesMenuNames[Types[i]]);
            }
        }
        
        // Draw the keybinding for the letters
        char ShortKey[2] = { '1' + i, 0 };
        CustomText->SetText(ShortKey);
        CustomText->Render(ix + IconW, iy + IconH);
    }
}

void g2DashboardController::Update(float dT)
{
    // ...
}

void g2DashboardController::WindowResizeEvent(int NewWidth, int NewHeight)
{
    // Save for top bar
    WindowWidth = NewWidth;
    
    // Window resized, force update
    SetTime();
}

void g2DashboardController::MouseHover(int x, int y)
{
    MouseX = x;
    MouseY = y;
}
 
void g2DashboardController::MouseClick(g2MouseButton button, g2MouseClick state, int x, int y)
{
    // Only check on release of left mouse
    if(button != g2MouseButton_Left || state != g2MouseClick_Up)
        return;
    
    // For each icon
    for(int i = 0; i < TypeCount; i++)
    {
        // Positions for icon background, top-icon needs to do + <dx, dy>
        int ix = IconOffsetX + IconSpaceX * (i % IconColCount);
        int iy = IconOffsetY + IconSpaceY * (i / IconColCount);
        
        // If the mouse is in this position, act as though the user is hovering over it
        if(x >= ix && x < ix + ButtonW && y >= iy && y < iy + ButtonH)
        {
            // Ignore clicking the first element if we are in selection mode
            if(State == DashboardState_Selection && i == 0)
                continue;
            
            // Alyways go to the root screen when canceling and cancel any volume selection
            else if(State != DashboardState_Root && i == TypeCount - 1)
            {
                State = DashboardState_Root;
                Selecting = false;
            }
            // If in the root menu, just change the state, else, start selection
            else if(State == DashboardState_Root)
            {
                State = DashboardState_Group;
                StateGroup = (UI_RootMenu)i;
            }
            // Else, actuallying doing some volume selection
            else if(State == DashboardState_Group)
            {
                State = DashboardState_Selection;
                Selecting = true;
                
                // Set the selection
                if(StateGroup == UI_RootMenu_Build)
                    Type.Building = (UI_BuildMenu)i;
                else if(StateGroup == UI_RootMenu_Designations)
                    Type.Designation = (UI_DesignationMenu)i;
                else if(StateGroup == UI_RootMenu_Stockpiles)
                    Type.Stockpile = (UI_StockpilesMenu)i;
                else if(StateGroup == UI_RootMenu_Zones)
                    Type.Zone = (UI_ZonesMenu)i;
            }
            // Else, user is commiting change
            else if(State == DashboardState_Selection && i == 0)
            {
                State = DashboardState_Root;
                Selecting = false;
                
                // Choose origin and volume
                Vector3<int> Origin = SelectStart;
                Vector3<int> Volume = SelectEnd - SelectStart + Vector3<int>(1, 1, 1);
                
                // Save into designations list
                if(StateGroup == UI_RootMenu_Build)
                    Designations->AddBuilding(Type.Building, Origin, Volume);
                else if(StateGroup == UI_RootMenu_Designations)
                    Designations->AddDesignation(Type.Designation, Origin, Volume);
                else if(StateGroup == UI_RootMenu_Stockpiles)
                    Designations->AddStockpile(Type.Stockpile, Origin, Volume);
                else if(StateGroup == UI_RootMenu_Zones)
                    Designations->AddZone(Type.Zone, Origin, Volume);
            }
        }
    }
}

void g2DashboardController::GetCollisionRect(int* Width, int* Height)
{
    // Trivial, just to catch all UI inputs
    *Width = GetWidth();
    *Height = GetHeight();
}
