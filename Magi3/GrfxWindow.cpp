/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "GrfxWindow.h"

/*** Internal private globals ***/

// Internal pointer to the only window instace
GrfxWindow *__WindowPtr = NULL;

/*** GrfxWindow functions ***/

GrfxWindow::GrfxWindow(const char *WindowName, int WindowWidth, int WindowHeight)
{
    // UtilAssert valid window name and save pos/size
    UtilAssert(WindowName != NULL, "Window name is null.");
    strcpy(Title, WindowName);
    
    // UtilAssert basic window size is large enough
    UtilAssert(WindowWidth > 0 && WindowHeight > 0, "Window size (%d, %d) is too small.", WindowWidth, WindowHeight);
    SizeWidth = WindowWidth;
    SizeHeight = WindowHeight;
    
    // Make sure we havent already created a window
    UtilAssert(__WindowPtr == NULL, "Only one Window may be created at a time.");
    __WindowPtr = this;
    
    // Default fullscreen and quit state
    IsFullscreen = false;
    
    // Defaults to 2D rendering and to activly clearing the buffer
    SetClearBool = true;
    NearPlane = 1.0f;
    
    // Set 3D camera properties
    Fovy = 60.0f;
    SetNearPlane(1.0f);
    SetFarPlane(1000.0f);
    
    // Set target framerate default speed to 60 FPS
    SetTargetFPS(60);
    
    // Create fake data for glutInit to work correctly (Must be called)
    int argc = 1;
    char targv[128] = "Fake data...";
    char* argv = targv;
    
    // Initialize glut and window mode, remove console if needed
    glutInit(&argc, &argv);
    
    // Double buffer w/ RGBA colors and z-depth turned on
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    
    // Center the window
    int SystemResWidth = glutGet(GLUT_SCREEN_WIDTH);
    int SystemResHeight = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowPosition(SystemResWidth / 2 - SizeWidth / 2, SystemResHeight / 2 - SizeHeight / 2);
    
    // Set the window size
    glutInitWindowSize(SizeWidth, SizeHeight);
    
    // Set the window to continue execution flow if closed via "x" (Part of freeGLUT)
    //glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    
    // Create window and save the initial title
    WindowHandle = glutCreateWindow(WindowName);
    strcpy(Title, WindowName);
    
    // Create a glui instance
    GLUIWindow = new Glui2("UITheme.cfg", __Idle, __Reshape, __KeyboardDown, __SpecialKeyboard, __Mouse, __PassiveMouse);
    glutDisplayFunc(__Display);
    glutKeyboardUpFunc(__KeyboardUp);
    
    /* ORIGINAL MAGI3 METHOD
    
    // Register the call back functions
    glutDisplayFunc(__Display);                // Rendering callback
    
    glutIdleFunc(__Idle);                    // Create the update callback
    glutReshapeFunc(__Reshape);                // Reshape window callback
    
    glutKeyboardFunc(__KeyboardDown);        // Keyboard event callback (When pressed)
    glutKeyboardUpFunc(__KeyboardUp);        // Keyboard event callback (When released)
    
    glutSpecialFunc(__SpecialKeyboard);        // Special keyboard event callback
    glutMouseFunc(__Mouse);                    // Mouse event callback
    glutMotionFunc(__DragMouse);            // Mouse drag movement
    glutPassiveMotionFunc(__PassiveMouse);    // Mouse movement callback
    
    */
    
    // Turn on alpha blending for textures
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.01f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_DEPTH_TEST);
    
    // Set the background color to black
    SetBackground();
    
    // Initialize to perspective-3D (Force from 2D to 3D)
    Is2D = true;
    Set2D(false);
    Use3DPerspective(true);
    
    // Initialize message
    printf("Window \"%s\" has been initialized.\n", Title);
    SetGrfxObjectName("Root GrfxObject (GrfxWindow)");
}

GrfxWindow::~GrfxWindow()
{
    // Release message
    printf("Window has been released.\n");
    
    Quit();
}

void GrfxWindow::SetWindowSize(int Width, int Height)
{
    // Resize the window
    glutReshapeWindow(Width, Height);
}

void GrfxWindow::GetWindowSize(int* Width, int* Height)
{
    if(Width != NULL)
        *Width = SizeWidth;
    if(Height != NULL)
        *Height = SizeHeight;
}

void GrfxWindow::SetFullScreen(bool Fullscreen)
{
    // Only change if requested
    if(IsFullscreen != Fullscreen)
    {
        // If we need to fullscreen
        if(Fullscreen == true)
        {
            // Save the window state
            WindowWidth = SizeWidth;
            WindowHeight = SizeHeight;
            glutFullScreen();
            
            // True fullscreen system
            /*
             // Sample full-screen "game mode" string generation
             // This would produce "800x600:32@60"
             // Fullscreen at 800 by 600 resolution, with bit depth of 32 bits, 60 Hz refresh
             char str[128];
             sprintf(str, "%dx%d:%d@%d", 640, 480, 16, 60);
             glutGameModeString(str);
             
             // Can we go into game mode?
             UtilAssert(glutGameModeGet(GLUT_GAME_MODE_POSSIBLE) != 0, "Could not enter full-screen mode.");
             
             // Note: I tried removing the part for the color mode, but that didn't do anything.
             glutEnterGameMode();
             
             // Register the call back functions
             glutDisplayFunc(__Display);                // Rendering callback
             glutIdleFunc(__Idle);                    // Create the update callback
             glutReshapeFunc(__Reshape);                // Reshape window callback
             
             // Set new background color
             SetBackground(1.0f, 0.8f, 0.6f);
             */
        }
        
        // Else, restore to standard window
        else if(Fullscreen == false)
        {
            // Apparently if we call any resized it pops us back to a standard view
            glutLeaveGameMode();
            glutReshapeWindow(WindowWidth, WindowHeight);
            glutPositionWindow(glutGet(GLUT_SCREEN_WIDTH) / 2 - WindowWidth / 2, glutGet(GLUT_SCREEN_HEIGHT) / 2 - WindowHeight / 2);
        }
        
        // Save the new state
        IsFullscreen = Fullscreen;
    }
}

bool GrfxWindow::IsFullScreen()
{
    return IsFullscreen;
}

void GrfxWindow::SetBackground(float r, float g, float b, float a)
{
    // Save the given values (Do a bounds check)
    bR = max(min(r, 1.0f), 0.0f);
    bG = max(min(g, 1.0f), 0.0f);
    bB = max(min(b, 1.0f), 0.0f);
    bA = max(min(a, 1.0f), 0.0f);
    
    // Set the clear color
    glClearColor(bR, bG, bB, bA);
}

void GrfxWindow::SetTitle(const char* WindowTitle)
{
    // Copy over and then load up on the next FPS event
    strcpy(Title, WindowTitle);
}

void GrfxWindow::Run()
{
    // Star the main cycle
    glutMainLoop();
}

void GrfxWindow::Quit()
{
    // Stop the main glut loop (Only part of freeGLUT)
    //glutLeaveMainLoop();
    
    // Hide window
    glutHideWindow();
    
    // Release the window
    glutDestroyWindow(WindowHandle);
    
    // Quit application
    exit(0);
}

void GrfxWindow::Use3DPerspective(bool UsePerspective)
{
    this->UsePerspective = UsePerspective;
}

void GrfxWindow::SetTargetFPS(int FrameRate)
{
    FPS = FrameRate;
}

void GrfxWindow::SetClear(bool Set)
{
    SetClearBool = Set;
}

void GrfxWindow::Set2D(bool Set)
{
    // If already in the same mode
    if(Is2D == Set)
        return;
    
    // Save new state
    Is2D = Set;
    
    // Force ortho or projection change event
    int Width, Height;
    GetWindowSize(&Width, &Height);
    __SetSize(Width, Height);
}

bool GrfxWindow::Get2D()
{
    return Is2D;
}

Glui2* GrfxWindow::GetGLUI()
{
    return GLUIWindow;
}

void GrfxWindow::SetFovy(float Set)
{
    Fovy = Set;
    
    // Force resize event
    int Width, Height;
    GetWindowSize(&Width, &Height);
    __SetSize(Width, Height);
}

float GrfxWindow::GetFovy()
{
    return Fovy;
}

void GrfxWindow::SetFarPlane(float newFar)
{
    FarPlane = newFar;
}

void GrfxWindow::SetNearPlane(float newNear)
{
    NearPlane = newNear;
}

float GrfxWindow::GetNearPlane()
{
    return NearPlane;
}

float GrfxWindow::GetFarPlane()
{
    return FarPlane;
}

void GrfxWindow::__Update(float dT)
{
    // Bug-fix on March 31st, 2011
    glutSetWindow(WindowHandle);
    
    // Start highres clocks
    static UtilHighresClock ExternalClock; // Measures the time between function call
    
    // Frame and general time counter
    static float dTime = (float)FPS / 1000.0f;
    static float totalTime = 0.0f;
    static float sleepTime = 0.0f;
    static int frameCount = 0;
    
    // Supress warnings and use dT
    dT = sleepTime + dTime;
    
    // Render all elements that have a priority below or equal to this object's priority
    int i;
    for(i = 0; i < ChildObjects.GetSize(); i++)
        if(ChildObjects[i]->GetPriority() <= GetPriority())
            ChildObjects[i]->__Update(dT);
        else
            break;
    
    // Update self
    Update(dT);
    
    // Call all children's render (Recursive)
    for(; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__Update(dT);
    
    // Post needed rendering
    glutPostRedisplay();
    
    // End external time
    ExternalClock.Stop();
    
    // Measure the cycle time
    dTime = ExternalClock.GetTime();
    
    // Calculate the ammount of time to sleep off
    sleepTime = (1.0f / (float)FPS) - dTime;
    if(sleepTime < 0.0f)
        sleepTime = 0.0f;
    if(dTime < 0.0f)
        dTime = 0.0;
    
    // Sleep off time if needed in microseconds (Convert from seconds to microseconds)
    // We also have to call our own sleep since Win32 and Unix sleeps are different
    UtilSleep(sleepTime); // Fractions of time
    totalTime += (sleepTime + dTime);
    
    // Print out FPS data
    if(frameCount++ > FPS)
    {
        // Change the title of the window to reflect the FPS
        char Buffer[256];
        sprintf(Buffer, "%s, %.3f / %d FPS - Magi3 Lite", Title, (float)frameCount / totalTime, FPS);
        glutSetWindowTitle(Buffer);
        
        totalTime = 0.0f;
        frameCount = 0;
    }
    
    // Start external time
    ExternalClock.Start();
}

void GrfxWindow::__Render()
{
    // Set the background color
    if(SetClearBool)
    {
        //glDepthMask(true);
        //glDepthFunc(GL_ALWAYS);
        //glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    
    // Render all elements that have a priority below or equal to this object's priority
    int i;
    for(i = 0; i < ChildObjects.GetSize(); i++)
        if(ChildObjects[i]->GetPriority() <= GetPriority())
            ChildObjects[i]->__Render();
        else
            break;
    
    // Render self
    Render();
    
    // Call all children's render (Recursive)
    for(; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__Render();
    
    // Render the glui
    GetGLUI()->Render();
    
    // Swap buffers
    glutSwapBuffers();
}

void GrfxWindow::__SetSize(int Width, int Height)
{
    // Set size
    SizeWidth = Width;
    SizeHeight = Height;
    
    // Apply needed openGL updates
    glViewport(0, 0, SizeWidth, SizeHeight);
    
    // Reset the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Change to 2D or 3D ortho/projection matrices
    if(Is2D)
    {
        // 2D projection
        gluOrtho2D(0.0, float(SizeWidth), float(SizeHeight), 0.0);
        glMatrixMode(GL_MODELVIEW);
    }
    else
    {
        // 3D projection
        if(UsePerspective)
            gluPerspective(Fovy, float(SizeWidth) / float(SizeHeight), NearPlane, FarPlane);
        else
            glOrtho(-float(SizeWidth) / Fovy, float(SizeWidth) / Fovy, -float(SizeHeight) / Fovy, float(SizeHeight) / Fovy, NearPlane, FarPlane);
        glMatrixMode(GL_MODELVIEW);
    }
    
    // Call all children's window size change (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__WindowResizeEvent(SizeWidth, SizeHeight);
    
    // Call overloaded function
    WindowResizeEvent(SizeWidth, SizeHeight);
}

void GrfxWindow::__Display()
{
    // Render the scene
    __WindowPtr->__Render();
}

void GrfxWindow::__Idle()
{
    // Update the scene
    // Note we send junk time, just to keep the function types consistent
    __WindowPtr->__Update(0.0);
}

void GrfxWindow::__Reshape(int width, int height)
{
    // Pass the new width and height into the window instance
    __WindowPtr->__SetSize(width, height);
}

void GrfxWindow::__KeyboardDown(unsigned char key, int x, int y)
{
    /*
     // Exit on case 27 (esc key)
     if(key == 27)
     {
     // Deletion will cause self release and exit
     delete __WindowPtr;
     }
     // Fullscreen swap
     else if(key == 'f')
     {
     __WindowPtr->SetFullScreen(!__WindowPtr->IsFullScreen());
     }
     */
    //printf("Keyboard press event, Key: '%c'\n", key);
    __WindowPtr->__KeyboardEventDown(key, x, y);
}

void GrfxWindow::__KeyboardUp(unsigned char key, int x, int y)
{
    //printf("Keyboard press event, Key: '%c'\n", key);
    __WindowPtr->__KeyboardEventUp(key, x, y);
}

void GrfxWindow::__SpecialKeyboard(int key, int x, int y)
{
    //printf("Special keyboard press event, Key: '%c'\n", key);
    __WindowPtr->__SpecialKeyboardEvent(key, x, y);
}

void GrfxWindow::__Mouse(int button, int state, int x, int y)
{
    //printf("Mouse press event, button: '%i', state: %i, x: %i, y: %i\n", button, state, x, y);        
    __WindowPtr->__MouseEvent(button, state, x, y);
}

void GrfxWindow::__DragMouse(int x, int y)
{
    //printf("Mouse drag event x: %i, y: %i\n", x, y); // Too verbose
    __WindowPtr->__DragMouseEvent(x, y);
}

void GrfxWindow::__PassiveMouse(int x, int y)
{
    //printf("Mouse passive event x: %i, y: %i\n", x, y); // Too verbose
    __WindowPtr->__PassiveMouseEvent(x, y);
}
