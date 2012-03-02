/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "g2ChatController.h"
#include "Entity.h"

// Have to still declare object
g2ChatController* g2ChatController::SelfHandle;

g2ChatController::g2ChatController(g2Controller* Parent, g2Theme* MainTheme)
: g2Controller(Parent, MainTheme)
{
    // Initialize the editable label
    LabelEdit = new g2LabelEdit(this, MainTheme);
    LabelEdit->SetShadow(false);
    LabelEdit->SetColor(255, 255, 255);
    
    // Initialize the label itself
    Label = new g2Label(this, MainTheme);
    Label->SetText(":> ");
    Label->SetShadow(true);
    Label->SetColor(255, 255, 255);
    
    // Set size
    int CompWidth, CompHeight;
    GetTheme()->GetComponentSize("ChatController", &CompWidth, &CompHeight);
    SetSize(CompWidth * 4, CompHeight * 4);
    
    // Default initial message (has to be on the heap, can't be on the stack)
    char* Header = new char[128];
    strcpy(Header, "Welcome to g2Console; write any commands and press [Enter] to execute");
    
    g2ChatController_Msg Message;
    Message.str = Header;
    Message.Focusable = false;
    
    ConsoleOut.push(Message);
    
    // No initial focus
    FocusState = false;
    
    // Save self as reference
    SelfHandle = this;
}

g2ChatController::~g2ChatController()
{
    // For each input
    while(!ConsoleIn.empty())
    {
        delete[] ConsoleIn.front();
        ConsoleIn.pop();
    }
    
    // For each output
    while(!ConsoleOut.empty())
    {
        delete[] ConsoleOut.front().str;
        ConsoleOut.pop();
    }
}

void g2ChatController::printf(const char* format, ...)
{
    // Convert into a valid buffer...
    va_list args;
    va_start(args, format);
    
    // Currently this is a naive approach because who knows how big
    // the output buffer will be? Thus, we will need to
    char buffer[1024];
    vsprintf(buffer, format, args);
    
    // Print to stdout; we don't use the classic "printf" since it
    // will call this class' implementation, leading to recursive calls
    fprintf(stdout, "g2ChatController: \"%s\"\n", buffer);
    
    // Copy and push
    char* newBuffer = new char[strlen(buffer) + 1];
    strcpy(newBuffer, buffer);
    
    g2ChatController_Msg Message;
    Message.str = newBuffer;
    Message.Focusable = false;
    
    SelfHandle->ConsoleOut.push(Message);
    
    // Too many in the console input? Release
    if(SelfHandle->ConsoleOut.size() > 64)
    {
        char* temp = SelfHandle->ConsoleOut.front().str;
        SelfHandle->ConsoleOut.pop();
        delete[] temp;
    }
    
    // Done with args
    va_end(args);
}

void g2ChatController::printf(Vector3<int> pos, const char* format, ...)
{
    // Convert into a valid buffer...
    va_list args;
    va_start(args, format);
    
    // Currently this is a naive approach because who knows how big
    // the output buffer will be? Thus, we will need to
    char buffer[1024];
    vsprintf(buffer, format, args);
    
    // Print to stdout; we don't use the classic "printf" since it
    // will call this class' implementation, leading to recursive calls
    fprintf(stdout, "g2ChatController: \"%s\"\n", buffer);
    
    // Copy and push
    char* newBuffer = new char[strlen(buffer) + 1];
    strcpy(newBuffer, buffer);
    
    g2ChatController_Msg Message;
    Message.str = newBuffer;
    Message.Focusable = true;
    Message.Position = pos;
    Message.IsEntity = false;
    
    SelfHandle->ConsoleOut.push(Message);
    
    // Too many in the console input? Release
    if(SelfHandle->ConsoleOut.size() > 64)
    {
        char* temp = SelfHandle->ConsoleOut.front().str;
        SelfHandle->ConsoleOut.pop();
        delete[] temp;
    }
    
    // Done with args
    va_end(args);
}

void g2ChatController::printf(Entity* EntityHandle, const char* format, ...)
{
    // Convert into a valid buffer...
    va_list args;
    va_start(args, format);
    
    // Currently this is a naive approach because who knows how big
    // the output buffer will be? Thus, we will need to
    char buffer[1024];
    vsprintf(buffer, format, args);
    
    // Print to stdout; we don't use the classic "printf" since it
    // will call this class' implementation, leading to recursive calls
    fprintf(stdout, "g2ChatController: \"%s\"\n", buffer);
    
    // Copy and push
    char* newBuffer = new char[strlen(buffer) + 1];
    strcpy(newBuffer, buffer);
    
    g2ChatController_Msg Message;
    Message.str = newBuffer;
    Message.Focusable = true;
    Message.IsEntity = true;
    Message.EntityHandle = EntityHandle;
    
    SelfHandle->ConsoleOut.push(Message);
    
    // Too many in the console input? Release
    if(SelfHandle->ConsoleOut.size() > 64)
    {
        char* temp = SelfHandle->ConsoleOut.front().str;
        SelfHandle->ConsoleOut.pop();
        delete[] temp;
    }
    
    // Done with args
    va_end(args);
}

void g2ChatController::gets(char* OutBuffer, int* Length)
{
    if(ConsoleIn.empty())
    {
        *Length = 0;
        OutBuffer[0] = '\0';
    }
    else
    {
        // Get from stack
        char* out = ConsoleIn.front();
        ConsoleIn.pop();
        
        // Copy to given buffer
        strncpy(OutBuffer, out, 1023);
        *Length = (int)strlen(OutBuffer);
        
        // Release internal buffer
        delete[] out;
    }
}

void g2ChatController::SetSize(int Width, int Height)
{
    // Save size
    this->Width = Width;
    this->Height = Height;
    
    // Update label positon
    int HeightOffset = Height - GetTheme()->GetCharacterHeight() * 2 + 2;
    LabelEdit->SetPos(5 + Label->GetWidth(), HeightOffset);
    LabelEdit->SetWidth(Width - 10 - Label->GetWidth());
    Label->SetPos(5, HeightOffset);
}

int g2ChatController::GetWidth()
{
    return Width;
}

int g2ChatController::GetHeight()
{
    return Height;
}

bool g2ChatController::GetFocus(Vector3<float>* FocusBlock)
{
    bool HasFocus = FocusState;
    FocusState = false;
    *FocusBlock = FocusPos;
    return HasFocus;
}

void g2ChatController::Update(float dT)
{
    // Deal with user input
    if(LabelEdit->UserReturned())
    {
        // Copy the buffer
        char* input = new char[strlen(LabelEdit->GetText()) + 1];
        strcpy(input, LabelEdit->GetText());
        
        // Clear out user input
        LabelEdit->SetText();
        
        // Push into our input stack
        ConsoleIn.push(input);
        
        // Check if either of the queues are too big
        while((int)ConsoleIn.size() > g2Console_MaxInputSize)
            ConsoleIn.pop();
    }
}

void g2ChatController::Render(int x, int y)
{
    // Background first
    DrawComponent("ChatController", x, y, Width, Height);
    
    // Cursor position (Pixel offsets to look better)
    int CharHeight = GetTheme()->GetCharacterHeight() + 2;
    
    // 2 Pixel offset from the left, start two rows up so
    // we can have it on the gray console background AND
    // allow space for the input
    const int OffsetX = 8;
    int cX = OffsetX;
    int cY = y + Height - CharHeight * 3;
    
    // Render each character in our output buffer
    int OutLength = (int)ConsoleOut.size();
    for(int i = 0; i < OutLength; i++)
    {
        // Get the latest text
        g2ChatController_Msg Message = ConsoleOut.front();
        char* Text = Message.str;
        ConsoleOut.pop();
        
        // Get the default color
        float TempR, TempG, TempB;
        TempR = TempG = TempB = 0;
        
        // Print to screen
        for(size_t j = 0; j < strlen(Text); j++)
        {
            // Get target char width
            int CharWidth;
            GetTheme()->GetCharacterSize(Text[j], &CharWidth);
            
            // Stop if off-screen
            if((cY - (OutLength - i) * CharHeight) <= y || (cX + CharWidth + OffsetX) >= Width)
                break;
            
            // Is this character a backslash?
            if(Text[j] == '\\')
            {
                // Attempt to read an integer
                int ColorID = -1;
                sscanf(Text + j + 1, "%d", &ColorID);
                if(ColorID >= 0 && ColorID < 16)
                {
                    // Change color and move ahead until non-numeric
                    do {
                        j++;
                    } while(Text[j] >= '0' && Text[j] <= '9');
                    g2Label::GetTemplateColor(ColorID, &TempR, &TempG, &TempB);
                }
            }
            
            // Draw the shadow first
            DrawCharacter(cX, cY - (OutLength - i) * CharHeight, 1, 1, TempR, TempG, TempB, 1, Text[j]);
            cX += CharWidth + 2;
        }
        
        // Reset x position
        cX = OffsetX;
        
        // Put back in
        ConsoleOut.push(Message);
    }
}

void g2ChatController::GetCollisionRect(int* Width, int* Height)
{
    // Post-back the size of this controller
    *Height = this->Height;
    *Width = this->Width;
}

void g2ChatController::MouseClick(g2MouseButton button, g2MouseClick state, int x, int y)
{
    // Only move camera if left button and down
    if(button == g2MouseButton_Left && state == g2MouseClick_Down)
    {
        // Cursor position (Pixel offsets to look better)
        int CharHeight = GetTheme()->GetCharacterHeight() + 2;
        int cY = CharHeight * 3;
        
        // For each message
        int OutLength = (int)ConsoleOut.size();
        for(int i = 0; i < OutLength; i++)
        {
            // Get the latest text
            g2ChatController_Msg Message = ConsoleOut.front();
            ConsoleOut.pop();
            
            // Top of the message string
            int top = Height - cY - (OutLength - i) * CharHeight;
            int bottom = top + CharHeight;
            
            // We intersecting this and it is focusable
            if(y >= top && y <= bottom && Message.Focusable)
            {
                // Move camera to focus on position
                FocusState = true;
                
                // If entity, get its position and post that instead
                if(Message.IsEntity)
                    FocusPos = Message.EntityHandle->GetPosition();
                else
                    FocusPos = Vector3itof(Message.Position) + 0.5f; // So we focus on the middle of the block
            }
            
            // Put back in
            ConsoleOut.push(Message);
        }
    }
}
