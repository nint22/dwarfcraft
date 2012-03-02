/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: GameRender.h/cpp
 Desc: A simple volume renderer; generates a VBO per layer
 of a given WorldContainer and attempts to optimize the VBO
 generation.
 
***************************************************************/

// Inclusion guard
#ifndef __GAMERENDER_H__
#define __GAMERENDER_H__

#include "Globals.h"
#include "MGrfx.h"
#include "WorldContainer.h"

#include "WorldGenerator.h"
#include "BackgroundView.h"
#include "UserInterface.h"
#include "WorldView.h"
#include "DesignationsView.h"
#include "ItemsView.h"
#include "Entities.h"
#include "StructsView.h"

class GameRender : public GrfxObject
{
public:
    
    // Constructor takes the window we will be bound to, and the Glui handle
    GameRender(GrfxObject* Parent, Glui2* GluiHandle);
    ~GameRender();
    
protected:
    
    // Render *all* the things (raises fists)
    void Render();
    
    // Overloaded update function
    void Update(float dT);
    
    // Overload window resize event
    void WindowResizeEvent(int NewWidth, int NewHeight);
    
    // Overload key-event down
    void KeyboardEventDown(unsigned char key, int x, int y);
    
    // Overload key-event up
    void KeyboardEventUp(unsigned char key, int x, int y);
    
    // Overloaded for mouse press/release events
    void MouseEvent(int button, int state, int x, int y);
    
    // Overloaded for dragging mouse across the screen
    void PassiveMouseEvent(int x, int y);
    
private:
    
    /*** Helper Functions ***/
    
    // Given a target we want to point to, return the camera source position
    // Which is based on the target, rotation angle, and pitch angle
    Vector3<float> GetCameraSource();
    
    /*** World Data & Renderables ***/
    
    // World size and height
    int WorldWidth, WorldHeight;
    
    // Background graphic
    BackgroundView* Background;
    
    // User interface
    UserInterface* WorldUI;
    
    // The main world volume (i.e. data)
    WorldContainer* WorldData;
    
    // The rendering mechanism
    WorldView* WorldRender;
    
    // Designations list
    DesignationsView* Designations;
    
    // Items list
    ItemsView* Items;
    
    // Structs list
    StructsView* Structs;
    
    // Entity list
    Entities* EntitiesList;
    
    /*** Camera Position and User Controls ***/
    
    // Where the camera is pointing from (source) and to (target)
    // Note that the target is what actually moves, while camera source
    // is just the rotated & pitched distance from the target
    Vector3<float> CameraTarget;
    
    // Camera rotation
    float CameraRotation;
    
    // Amount of pitch the camera currently has
    // This is an offset, not the actual full pitch from the horizon
    float CameraPitchOffset;
    
    // Ammount that the user wants to zoom in and out (Ranges from 1 to 180)
    float CameraZoom;
    
    // User key press
    bool KeyUp, KeyDown, KeyLeft, KeyRight;
    
    // World depth and zoom (note the stall values)
    bool KeyZoomIn, KeyZoomOut;
    
    // User's mouse dragging states
    bool MouseDragging, MouseRotation, ShiftRotating, CtrlRotating;
    
    // The initial dragging position of a mouse-click event
    int MouseStartX, MouseStartY;
    
    /*** Misc. ***/
    
    // Active render style
    GameRender_RenderStyle RenderStyle;
    
    // Window size
    int WindowWidth, WindowHeight;
    
    // Flags for specific rendering (Debug)
    bool RenderDesignationsFlag;
    
    // World cutoff: the last floor that we render, everything including it and below is rendered
    int LayerCutoff;
};

// End of inclusion guard
#endif
