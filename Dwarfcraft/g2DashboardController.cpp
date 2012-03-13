/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "g2DashboardController.h"

// Helper function to map icons to designation types
static inline DesignationType GetDesignationType(IconType Type)
{
    DesignationType Result = DesignationType_Armory; // Default
    
    // Case for fast mapping
    switch(Type)
    {
        case IconType_Mine:
            Result = DesignationType_Mine; break;
        case IconType_Fill:
            Result = DesignationType_Fill; break;
        case IconType_Flood:
            Result = DesignationType_Flood; break;
        case IconType_Rubbish:
            Result = DesignationType_Rubbish; break;
        case IconType_Food:
            Result = DesignationType_Food; break;
        case IconType_Crafted:
            Result = DesignationType_Crafted; break;
        case IconType_RawResources:
            Result = DesignationType_RawResources; break;
        case IconType_Ingots:
            Result = DesignationType_Ingots; break;
        case IconType_Grave:
            Result = DesignationType_Grave; break;
        case IconType_Farm:
            Result = DesignationType_Farm; break;
        case IconType_Wood:
            Result = DesignationType_Wood; break;
        case IconType_Forage:
            Result = DesignationType_Forage; break;
        case IconType_Protect:
            Result = DesignationType_Protect; break;
        case IconType_Barracks:
            Result = DesignationType_Barracks; break;
        case IconType_Hall:
            Result = DesignationType_Hall; break;
        case IconType_Armory:
            Result = DesignationType_Armory; break;
        
        // Incorect
        case IconType_Construct:
        case IconType_Storage:
        case IconType_Collect:
        case IconType_Military:
        case IconType_Accept:
        case IconType_Cancel:
            UtilAbort("Unable to map the given IconType to a DesignationType!");
    }
    
    // Designation found
    return Result;
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
    
    // Get the initial size of the icon (Armory is just one of any regular icon)
    GetTheme()->GetComponentSize(GetIconName(IconType_Armory), &IconW, &IconH);
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
        IconType Types[4] = {IconType_Construct, IconType_Storage, IconType_Collect, IconType_Military};
        Render(x, y, Types, 4);
    }
    // In a group
    else if(State == DashboardState_Group)
    {
        // Construction
        if(StateGroup == DesignationGroup_Construct)
        {
            IconType Types[4] = {IconType_Mine, IconType_Fill, IconType_Flood, IconType_Cancel};
            Render(x, y, Types, 4);
        }
        else if(StateGroup == DesignationGroup_Storage)
        {
            IconType Types[7] = {IconType_Rubbish, IconType_Food, IconType_Crafted, IconType_RawResources, IconType_Ingots, IconType_Grave, IconType_Cancel};
            Render(x, y, Types, 7);
        }
        else if(StateGroup == DesignationGroup_Collect)
        {
            IconType Types[4] = {IconType_Farm, IconType_Wood, IconType_Forage, IconType_Cancel};
            Render(x, y, Types, 4);
        }
        else if(StateGroup == DesignationGroup_Military)
        {
            IconType Types[5] = {IconType_Protect, IconType_Barracks, IconType_Hall, IconType_Armory, IconType_Cancel};
            Render(x, y, Types, 5);
        }
    }
    // Actively selecting, only show a commit and reject button
    else if(State == DashboardState_Selection)
    {
        // We can cast directly from a DesignationType to an IconType, they are the same (initial) offsets
        IconType Types[3] = {(IconType)StateType, IconType_Accept, IconType_Cancel};
        Render(x, y, Types, 3);
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
        DrawComponent(GetIconName(Types[i]), ix + dx, iy + dy, IconW, IconH);
        
        // If the mouse is in this position, act as though the user is hovering over it
        // and draw a label of the text as well
        if(MouseX >= ix - x && MouseX < ix - x + ButtonW && MouseY >= iy - y && MouseY < iy - y + ButtonH)
        {
            // Only draw the hover if not the designation type when in selection mode
            if(State != DashboardState_Selection || i != 0)
                DrawComponent("IconButton_Hover", ix, iy, ButtonW, ButtonH);
            
            // Button names:
            
            // Cancel button if non-root and last element
            if(State != DashboardState_Root && i == TypeCount - 1)
                ClockText->SetText("Cancel");
            
            // Commit button if selection and second-last element
            else if(State == DashboardState_Selection && i == 1)
                ClockText->SetText("Make Designation");
            
            // Root view
            else if(State == DashboardState_Root)
            {
                if(i == 0)
                    ClockText->SetText("Construct");
                else if(i == 1)
                    ClockText->SetText("Storage");
                else if(i == 2)
                    ClockText->SetText("Collect");
                else
                    ClockText->SetText("Military");
            }
            
            // All other views are just diret icon names
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
                StateGroup = (DesignationGroup)i;
            }
            // Else, actuallying doing some volume selection
            else if(State == DashboardState_Group)
            {
                State = DashboardState_Selection;
                StateType = GetDesignationType(Types[i]);
                Selecting = true;
            }
            // Else, user is commiting change
            else if(State == DashboardState_Selection && i == 1)
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
