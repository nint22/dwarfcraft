/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: g2ChatController.h/cpp
 Desc: An interactive console for user input; mostly used for
 reading events posted by the game.
 
***************************************************************/

// Inclusion guard
#ifndef __G2CHATCONTROLLER_H__
#define __G2CHATCONTROLLER_H__

#include <Glui2/Glui2.h>
#include "Vector3.h"

// Foward declare Entity, to prevent recursive-inclusion
class Entity;

// Maximum input length
static const int g2ChatController_MaxInputSize = 64;

// Chat controller message (string with other data as needed)
struct g2ChatController_Msg
{
    // String (needs to be explicitly released from the heap)
    char* str;
    
    // True if we have a vector we can focus the camera on
    bool Focusable;
    Vector3<int> Position;
    
    // If this is true, instead of a position, we use an entity pointer
    bool IsEntity;
    Entity* EntityHandle;
};

// Chat implementation
class g2ChatController : public g2Controller
{
public:
    
    // Console constructor
    g2ChatController(g2Controller* Parent, g2Theme* MainTheme);
    
    // Release the i/o buffers
    ~g2ChatController();
    
    // Print to the buffer (global access)
    static void printf(const char* format, ...);
    
    // Print to the buffer (global access); if user clicks, focuses on position
    static void printf(Vector3<int> pos, const char* format, ...);
    
    // Print to the buffer (global access); if user clicks, focuses on entity
    static void printf(Entity* EntityHandle, const char* format, ...);
    
    // Read off the latest string command the user has typed in; based on FIFO order
    // May return null if there is nothing in the buffer; will copy over 'length'
    // chars into the given buffer; maximum input is 1024 x sizeof(char), if
    // there is no imput, length will be 0
    void gets(char* OutBuffer, int* Length);
    
    // Set the output size
    void SetSize(int Width, int Height);
    
    // Get the width / height of the controller
    int GetWidth();
    int GetHeight();
    
    // Has a position the user wants to focus on
    // If true, a focusable position is posted in the vector
    // Note: this is a post-state function; calling it a second time may not
    // post the focusable block position
    bool GetFocus(Vector3<float>* FocusBlock);
    
protected:
    
    // Update over time
    void Update(float dT);
    
    // Render
    void Render(int pX, int pY);
    
    // Define controller geometry
    void GetCollisionRect(int* Width, int* Height);
    
    // Callback for when a user clicks on a focusable event
    void MouseClick(g2MouseButton button, g2MouseClick state, int x, int y);
    
private:
    
    // Active width and height
    int Width, Height;
    
    // User input handler
    g2LabelEdit* LabelEdit;
    
    // The label infront of the edit
    g2Label* Label;
    
    // Input and output queues
    // Note that the out console might also retain
    // a copy of the input; since the user should
    // see what they write
    std::queue< char* > ConsoleIn;
    std::queue< g2ChatController_Msg > ConsoleOut;
    
    // Main self-handle
    static g2ChatController* SelfHandle;
    
    // Used for focusing
    bool FocusState;
    Vector3<float> FocusPos;
};

// End of inclusion guard
#endif
