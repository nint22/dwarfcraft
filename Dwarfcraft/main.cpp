/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Main.cpp
 Desc: Main application entry point for DwarfCraft.
 
***************************************************************/

#include "MainView.h"

// Main application entry point
int main (int argc, const char * argv[])
{
    // Initialize application
    MainView Client;

    // Start main loop
    Client.Run();
    
    // Done
    return 0;
}
