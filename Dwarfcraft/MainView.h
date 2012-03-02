/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: MainView.h/cpp
 Desc: The minimal implementation for the main game screen which
 is two-parts: the rendered game scene, and all UI controllers.
 
***************************************************************/

#ifndef __MAINVIEW_H__
#define __MAINVIEW_H__

#include "GrfxWindow.h"
#include "GameRender.h"

class MainView : public GrfxWindow
{
public:
    
    // Standard constructor and destructor
    MainView();
    ~MainView();
    
protected:
    
    // Overloaded to accept "esc" for exit event
    void KeyboardEventDown(unsigned char key, int x, int y);
    
private:
    
    // Game renderer
    GameRender* Game;
};

#endif