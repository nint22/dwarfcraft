/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "g2DashboardController.h"

// Internal helper function to give the coordinates of a given designation icon name
static inline void GetDesignationIcon(DesignationType Type, char* DesignationName)
{
    // Create the name
    sprintf(DesignationName, "Icon%s", DesignationNames[Type]);
}

g2DashboardController::g2DashboardController(g2Controller* Owner, g2Theme* MainTheme)
:g2Controller(Owner, MainTheme)
{
    // Set size
    int CompWidth, CompHeight;
    GetTheme()->GetComponentSize("DashboardController", &CompWidth, &CompHeight);
    SetSize(CompWidth * 4, CompHeight * 4);
    
    // Allocate the label's text
    ClockText = new g2Label(this, GetTheme());
    
    // Allocate a custom text renderer without management
    CustomText = new g2Label(NULL, MainTheme);
    
    // Regular icon size
    char IconName[64];
    GetDesignationIcon(DesignationType_Mine, IconName);
    
    // Get the initial size of the icon
    GetTheme()->GetComponentSize(IconName, &IconW, &IconH);
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
    TypeCount = 0;
    State = DashboardState_Root;
    
    // Not currently selecting
    Selecting = false;
}

g2DashboardController::~g2DashboardController()
{
    
}

void g2DashboardController::SetDesignationsList(DesignationsView* Designations)
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
    
    // Render default 4 designation types
    if(State == DashboardState_Root)
    {
        DesignationType Types[4] = {DesignationType_Mine, DesignationType_RawResources, DesignationType_Farm, DesignationType_Protect};
        Render(x, y, Types, 4);
    }
    // In a group
    else if(State == DashboardState_Group)
    {
        // Construction
        if(StateGroup == DesignationGroup_Construct)
        {
            DesignationType Types[3] = {DesignationType_Mine, DesignationType_Fill, DesignationType_Flood};
            Render(x, y, Types, 3);
        }
        else if(StateGroup == DesignationGroup_Storage)
        {
            DesignationType Types[6] = {DesignationType_Rubbish, DesignationType_Food, DesignationType_Crafted, DesignationType_RawResources, DesignationType_Ingots, DesignationType_Grave};
            Render(x, y, Types, 6);
        }
        else if(StateGroup == DesignationGroup_Collect)
        {
            DesignationType Types[3] = {DesignationType_Farm, DesignationType_Wood, DesignationType_Forage};
            Render(x, y, Types, 3);
        }
        else if(StateGroup == DesignationGroup_Military)
        {
            DesignationType Types[4] = {DesignationType_Protect, DesignationType_Barracks, DesignationType_Hall, DesignationType_Armory};
            Render(x, y, Types, 4);
        }
    }
    // Actively selecting, only show a commit and reject button
    else if(State == DashboardState_Selection)
    {
        DesignationType Types[1] = {StateType};
        Render(x, y, Types, 1);
    }
    // Else, no such group
    else
        UtilAbort("This sub-menu does not exist");
}

void g2DashboardController::Render(int x, int y, DesignationType* Types, int TypeCount)
{
    // Copy the designations
    //memcpy(this->Types, Types, sizeof(DesignationType) * TypeCount);
    for(int i = 0; i < TypeCount; i++)
        this->Types[i] = Types[i];
    this->TypeCount = TypeCount;
    
    // Regular icon size
    char IconName[64];
    
    // For each type
    for(int i = 0; i <= TypeCount; i++)
    {
        // Positions for icon background, top-icon needs to do + <dx, dy>
        int ix = x + IconOffsetX + IconSpaceX * (i % IconColCount);
        int iy = y + IconOffsetY + IconSpaceY * (i / IconColCount);
        
        // If we are at the end, and this is the default group, don't draw a "back" icon
        if(State == DashboardState_Root && i == TypeCount)
            continue;
        
        // If we are at the end, and this is not the default group, append a "back" icon
        else if(i == TypeCount)
            strcpy(IconName, "IconCancel");
        
        // Else, regular icon
        else
            GetDesignationIcon(Types[i], IconName);
        
        // Background image
        DrawComponent("IconButton", ix, iy, ButtonW, ButtonH);
        
        // Draw icon
        DrawComponent(IconName, ix + dx, iy + dy, IconW, IconH);
        
        // If the mouse is in this position, act as though the user is hovering over it
        // and draw a label of the text as well
        if(MouseX >= ix - x && MouseX < ix - x + ButtonW && MouseY >= iy - y && MouseY < iy - y + ButtonH)
        {
            DrawComponent("IconButton_Hover", ix, iy, ButtonW, ButtonH);
            if(i == TypeCount)
                ClockText->SetText("Cancel");
            else
                ClockText->SetText(DesignationNames[Types[i]]);
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
    
    // If we are not in the root menu, check +1 for the cancel button
    if(State != DashboardState_Root)
        TypeCount++;
    
    // For each icon
    for(int i = 0; i < TypeCount; i++)
    {
        // Positions for icon background, top-icon needs to do + <dx, dy>
        int ix = IconOffsetX + IconSpaceX * (i % IconColCount);
        int iy = IconOffsetY + IconSpaceY * (i / IconColCount);
        
        // If the mouse is in this position, act as though the user is hovering over it
        if(x >= ix && x < ix + ButtonW && y >= iy && y < iy + ButtonH)
        {
            // Alyways go to the root screen when canceling and cancel any volume selection
            if(State != DashboardState_Root && i == TypeCount - 1)
            {
                State = DashboardState_Root;
                Selecting = false;
            }
            // If in the root menu, just change the state, else, start selection
            else if(State == DashboardState_Root)
            {
                State = DashboardState_Group;
                StateGroup = (DesignationGroup)i;
            }
            // Else, actuallying doing some volume selection
            else if(State == DashboardState_Group)
            {
                State = DashboardState_Selection;
                StateType = Types[i];
                Selecting = true;
            }
            // Else, user is commiting change
            else if(State == DashboardState_Selection && i == 0)
            {
                State = DashboardState_Root;
                Selecting = false;
                
                // Save into designations list
                Designations->AddDesignation(StateType, SelectStart, SelectEnd - SelectStart + Vector3<int>(1, 1, 1));
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
