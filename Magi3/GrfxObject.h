/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: GrfxWindow.cpp/h
 Desc: The most basic renderable object that provides several
 properties. It is managed by a parent GrfxObject, which is
 ultimitely managed by a scene found within a GrfxContainer.
 Ordering is based from -inf to +inf in terms of access order.
 Note that these objects default to 3D objects unless set to 2D
 objects by setting via "Set2D".
 
 If a class were to inherit this class, make sure to call the
 "SetGrfxObjectName("[your class name]")" member function so
 that it helps with debugging and self-identification.
 
***************************************************************/

// Inclusion guard
#ifndef __GRFXOBJECT_H_
#define __GRFXOBJECT_H_

// Includes
#include "MUtil.h"
#include "PriorityQueue.h"

// Base class for a renderable object
class GrfxObject
{

public:
    
    // Constructor, adds self to parent, default priority is 0, and defaults to self-register
    // The boolean param should only be used by the container class
    GrfxObject(GrfxObject* Parent = NULL, int NewPriority = 0, bool SelfRegister = true);
    
    // Destructor, removes self and children from parent
    // We need to check: I don't think making this virtual is a smart idea
    virtual ~GrfxObject();
    
    /*** GrfxWindow System Updates ***/
    
    // Update object
    virtual void Update(float dT);
    
    // Render object
    virtual void Render();
    
    // Window resize event
    virtual void WindowResizeEvent(int NewWidth, int NewHeight);
    
    /*** GrfxWindow Input Device Events ***/
    
    // Glut keyboard event callback (When pressed)
    virtual void KeyboardEventDown(unsigned char key, int x, int y);
    
    // Glut keyboard event callback (When up)
    virtual void KeyboardEventUp(unsigned char key, int x, int y);
    
    // Glut special keyboard event callback
    virtual void SpecialKeyboardEvent(int key, int x, int y);
    
    // Glut mouse event callback
    virtual void MouseEvent(int button, int state, int x, int y);
    
    // Glut mouse drag event callback
    virtual void DragMouseEvent(int x, int y);
    
    // Glut passive mouse event callback
    virtual void PassiveMouseEvent(int x, int y);
    
    /*** Render Options ***/
    
    // Set visibility
    void SetVisibility(bool Visible);
    
    // Get visability
    bool GetVisibility();
    
    // Set enabled state (__Update never gets called if true)
    void SetEnabled(bool Enabled);
    
    // Get enabled state
    bool GetEnabled();
    
    // Set color
    void SetColor(float r, float g, float b);
    
    // Set alpha value
    void SetAlpha(float NewAlpha);
    
    // Get alpha value
    float GetAlpha();
    
    // Set priority
    void SetPriority(int NewPriority);
    
    // Get priority
    int GetPriority();
    
    // Internal object-map printing function; for debugging
    void PrintSceneMap(int depth = 0);
    
protected:
    
    // Set 2D render state (Set the active rendering state to 2D)
    virtual void Set2D(bool State);
    
    // Get 2D render state (Returns true if we are activly rendering 2D content)
    virtual bool Get2D();
    
    // Accessor to parent pointer
    virtual GrfxObject* GetParentObject();
    
    // Accessor to this object's child objects list
    virtual PriorityQueue< GrfxObject* >* GetChildObjects();
    
    // Set the current "GrfxObjectName" hint string to the given string
    void SetGrfxObjectName(const char* Name);
    
    // Object color, from 0.0f to 1.0f per chanel
    float R, G, B;
    
    // Alpha value, from 0.0f to 1.0f
    float Alpha;
    
    // Visable/Renderable, propagates down to children objects
    bool IsVisible;
    
    // Enabled state, propagates down to children update functions
    bool IsEnabled;
    
private:
    
    /*** Managed Object Updating ***/
    
    // Internal update function
    virtual void __Update(float dT);
    
    // Internal render function
    virtual void __Render();
    
    // Window resize event
    virtual void __WindowResizeEvent(int NewWidth, int NewHeight);
    
    // Glut keyboard event callback (When pressed)
    virtual void __KeyboardEventDown(unsigned char key, int x, int y);
    
    // Glut keyboard event callback (When up)
    virtual void __KeyboardEventUp(unsigned char key, int x, int y);
    
    // Glut special keyboard event callback
    virtual void __SpecialKeyboardEvent(int key, int x, int y);
    
    // Glut mouse event callback
    virtual void __MouseEvent(int button, int state, int x, int y);
    
    // Glut mouse drag event callback
    virtual void __DragMouseEvent(int x, int y);
    
    // Glut passive mouse event callback
    virtual void __PassiveMouseEvent(int x, int y);
    
    /*** Managed Object Properties ***/
    
    // Child objects list
    PriorityQueue< GrfxObject* > ChildObjects;
    
    // Parent object pointer
    GrfxObject* ParentObject;
    
    // Priority level for rendering order
    int Priority;
    
    // Simple one-word name of the class; a hint used in printing the priority map
    char GrfxObjectName[64];
    
    // List GrfxWindow as an accesable class (For internal access)
    // This is because the GrfxWindow should always be the main root node
    friend class GrfxWindow;
};

#endif
