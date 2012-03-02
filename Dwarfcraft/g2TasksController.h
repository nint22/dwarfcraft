/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: g2TasksController.h/cpp
 Desc: A custom g2Controller; renders all active tasks and
 their progress. Can be scrolled smoothly.
 
***************************************************************/

// Inclusion guard
#ifndef __G2TASKSCONTROLLER_H__
#define __G2TASKSCONTROLLER_H__

#include <Glui2/glui2.h>

class g2TasksController : public g2Controller
{
public:
    
    // Standard constructor / destructor
    g2TasksController(g2Controller* Owner, g2Theme* MainTheme);
    ~g2TasksController();
    
    // Set size of controller
    void SetSize(int Width, int Height);
    
    // Get the width, height of the controller
    int GetWidth();
    int GetHeight();
    
protected:
    
    // Render a bar group
    void Render(int x, int y);
    
private:
    
    // Target height
    int Width, Height;
    
};

#endif
