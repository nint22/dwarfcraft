/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "MainView.h"

MainView::MainView()
: GrfxWindow(__DWARFCRAFT_VERSION__, 1200, 800)
{
    // Initialize special OpenGL features
    UtilInit();
    
    // Prepare game (register self and pass the Glui handle)
    Game = new GameRender(this, GetGLUI());
}

MainView::~MainView()
{
    // Nothing to release yet
}

void MainView::KeyboardEventDown(unsigned char key, int x, int y)
{
    // If key is "esc", quit application
    if(key == 27)
        exit(0);
}
