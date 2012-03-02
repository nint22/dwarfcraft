/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: GrfxWindow.cpp/h
 Desc: A renderable window with input handles. To use, simply
 inherit the class and overload appropriate functions you want
 to access at run-time. Based on GLUT for window and input
 management. Once the window is created, call the instance's
 static function "Run()" to being rendering. After calling this
 function, nothing is returned, though your instance's destructor
 will be called before exiting.
 
 March 31st 2011: Added interfacing code with GLUI so that I can
 register my GLUT window and enable GLUI event-handling.
 
***************************************************************/

// Inclusion guard
#ifndef __GRFXWINDOW_H_
#define __GRFXWINDOW_H_

// Includes
#include "MUtil.h"
#include "GrfxObject.h"

// A renderable window with overloadable functionality
class GrfxWindow : public GrfxObject
{
public:
    
    /*** General Window Functions ***/
    
    // Constructor
    GrfxWindow(const char *WindowName, int WindowWidth, int WindowHeight);
    
    // Destructor
    // This is virtual, though don't overload it, and if you do, call it explicitly
    virtual ~GrfxWindow();
    
    // Set window size
    void SetWindowSize(int Width, int Height);
    
    // Get window size
    void GetWindowSize(int* Width, int* Height);
    
    // Fullscreen or standard window mode
    void SetFullScreen(bool Fullscreen);
    
    // Returns true if fullscreened
    bool IsFullScreen();
    
    // Set background color (Defaults to black)
    void SetBackground(float r = 0.0, float g = 0.0, float b = 0.0, float a = 1.0);
    
    // Set window title
    void SetTitle(const char* WindowTitle);
    
    // Run the current graphics loop (Overloaded "Update" and "Render" functions are called)
    void Run();
    
    // Quit the current window; Calls the destructor
    void Quit();
    
    // Additional code added by Jeremy Bridon on October 26th
    // When rendering in 3D, use this to set true or false to use
    // either the perspective projection matrix or the orthogonal (isometric)
    // projection matrix
    void Use3DPerspective(bool UsePerspective);
    
    /*** Managed internal rendering system ***/
    
    // Set the target FPS (Frames per second) refresh rate
    void SetTargetFPS(int FrameRate);
    
    // Set to true if turning on te backbuffer clearing (On by default)
    void SetClear(bool Set);
    
    // Set active rendering state (i.e. true when rendering in 2D, false in 3D)
    void Set2D(bool Set);
    
    // Get active rendering state (i.e. true when rendering in 2D, false in 3D)
    bool Get2D();    
    
    // Accessor to the active main-window GLUI pointer
    Glui2* GetGLUI();
    
    // Set fovy (60 by default)
    void SetFovy(float Set);
    
    // Returns field of view
    float GetFovy();
    
    // Set near plane distance
    void SetNearPlane(float near);
    
    // Get near plane distance
    float GetNearPlane();
    
    // Set far plane
    void SetFarPlane(float far);
    
    // get far plane
    float GetFarPlane();
    
private:
    
    /*** Internal Special Overloaded function versions ***/
    
    // Internal update function
    void __Update(float dT);
    
    // Internal render function
    void __Render();
    
    // Set the new window size; does internal changes to OpenGL matrices
    void __SetSize(int Width, int Height);
    
    /*** Glut Internals & Callbacks***/
    
    // Glut display function callback
    static void __Display();
    
    // Glut idle function callback
    static void __Idle();
    
    // Glut window resize callback
    static void __Reshape(int width, int height);
    
    // Glut keyboard event callback (When pressed)
    static void __KeyboardDown(unsigned char key, int x, int y);
    
    // Glut keyboard event callback (When up)
    static void __KeyboardUp(unsigned char key, int x, int y);
    
    // Glut special keyboard event callback
    static void __SpecialKeyboard(int key, int x, int y);
    
    // Glut mouse event callback
    static void __Mouse(int button, int state, int x, int y);
    
    // Glut mouse drag callback
    static void __DragMouse(int x, int y);
    
    // Glut passive mouse event callback
    static void __PassiveMouse(int x, int y);
    
    /*** Internal Data ***/
    
    // Window size
    int SizeWidth, SizeHeight;
    int WindowWidth, WindowHeight; // The pre-fullscreen size
    
    // Window title
    char Title[256];
    
    // Window handle ID (Onlye one window at a time)
    int WindowHandle;
    
    // Is fullscreen state
    bool IsFullscreen;
    
    // Background color
    float bR, bG, bB, bA;
    
    // Define the frames per second (Defaults to 60 FPS)
    int FPS;
    
    // Set to true if we want to clear the backbuffer
    bool SetClearBool;
    
    // Set to true if rendering in 2D and thus need ortho projection
    bool Is2D;
    
    // The fovy used internally for 3D rendering
    float Fovy;
    
    // Near and far plane
    float NearPlane, FarPlane;
    
    // Active-scene GLUI handle
    Glui2* GLUIWindow;
    
    // See "SetViewRatio(...)" for details
    bool UsePerspective;
};

#endif
