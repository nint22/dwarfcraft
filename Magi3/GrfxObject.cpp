/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "GrfxObject.h"

GrfxObject::GrfxObject(GrfxObject* Parent, int NewPriority, bool SelfRegister)
{
    // Save the parent
    ParentObject = Parent;
    
    // Default values
    IsVisible = true;                   // Defaults to visible
    IsEnabled = true;                   // Defaults to enabled
    SetColor(1.0f, 1.0f, 1.0f);         // Defaults to white / full channels
    Alpha = 1.0f;                       // Defaults to fully-seen
    
    // Add self to parent
    Priority = NewPriority;
    
    // Only self register if needed
    if(SelfRegister && GetParentObject() != NULL)
        GetParentObject()->GetChildObjects()->Insert(Priority, this);
    
    // Default name to GrfxObject
    SetGrfxObjectName("GrfxObject");
}

GrfxObject::~GrfxObject()
{
    // Release all children objects
    // This is done automatically since the ChildObjects list pops off the stack
    
    // Only release the parent if 
    if(ParentObject != NULL)
        GetParentObject()->GetChildObjects()->Remove(this);
}

void GrfxObject::Update(float dT)
{
    // To be overloaded...
}

void GrfxObject::Render()
{
    // To be overloaded...
}

void GrfxObject::WindowResizeEvent(int NewWidth, int NewHeight)
{
    // To be overloaded...
}

void GrfxObject::KeyboardEventDown(unsigned char key, int x, int y)
{
    // To be overloaded...
}

void GrfxObject::KeyboardEventUp(unsigned char key, int x, int y)
{
    // To be overloaded...
}

void GrfxObject::SpecialKeyboardEvent(int key, int x, int y)
{
    // To be overloaded...
}

void GrfxObject::MouseEvent(int button, int state, int x, int y)
{
    // To be overloaded...
}

void GrfxObject::DragMouseEvent(int x, int y)
{
    // To be overloaded...
}

void GrfxObject::PassiveMouseEvent(int x, int y)
{
    // To be overloaded...
}

void GrfxObject::SetVisibility(bool Visible)
{
    IsVisible = Visible;
}

bool GrfxObject::GetVisibility()
{
    return IsVisible;
}

void GrfxObject::SetEnabled(bool Enabled)
{
    IsEnabled = Enabled;
}

bool GrfxObject::GetEnabled()
{
    return IsEnabled;
}

void GrfxObject::SetColor(float r, float g, float b)
{
    // Save the color
    R = r;
    G = g;
    B = b;
}

void GrfxObject::SetAlpha(float NewAlpha)
{
    Alpha = NewAlpha;
}

float GrfxObject::GetAlpha()
{
    return Alpha;
}

void GrfxObject::SetPriority(int NewPriority)
{
    // Save priority
    Priority = NewPriority;
    
    // Change priority
    if(GetParentObject() != NULL)
        GetParentObject()->GetChildObjects()->ChangePriority(Priority, this);
}

int GrfxObject::GetPriority()
{
    return Priority;
}

GrfxObject* GrfxObject::GetParentObject()
{
    return ParentObject;
}

void GrfxObject::Set2D(bool State)
{
    GetParentObject()->Set2D(State);
}

bool GrfxObject::Get2D()
{
    return GetParentObject()->Get2D();
}

PriorityQueue< GrfxObject* >* GrfxObject::GetChildObjects()
{
    return &ChildObjects;
}

void GrfxObject::SetGrfxObjectName(const char* Name)
{
    UtilAssert(Name != NULL && strlen(Name) < 64, "Given GrfxObject name is either null or too large.");
    strcpy(GrfxObjectName, Name);
}

void GrfxObject::PrintSceneMap(int depth)
{
    // Print the number of dashes we need
    for(int i = 0; i < depth; i++)
        printf(" ");
    printf("|- %s (Priority: %d, Visible: %c, Alpha: %.2f)\n", GrfxObjectName, Priority, IsVisible ? 'T' : 'F', Alpha);
    
    // Print the child nodes (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->PrintSceneMap(depth + 1);
}

void GrfxObject::__Update(float dT)
{
    // Don't update children if not visible
    if(!IsVisible)
        return;
    
    // Don't update if not enabled
    if(!IsEnabled)
        return;
    
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
}

void GrfxObject::__Render()
{
    // Don't render children if not visible
    if(!IsVisible)
        return;
    
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
}

void GrfxObject::__WindowResizeEvent(int NewWidth, int NewHeight)
{
    // Call all children's related event functions (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__WindowResizeEvent(NewWidth, NewHeight);
    
    // Call this object's related event function
    WindowResizeEvent(NewWidth, NewHeight);
}

void GrfxObject::__KeyboardEventDown(unsigned char key, int x, int y)
{
    // Call all children's related event functions (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__KeyboardEventDown(key, x, y);
    
    // Call this object's related event function
    KeyboardEventDown(key, x, y);
}

void GrfxObject::__KeyboardEventUp(unsigned char key, int x, int y)
{
    // Call all children's related event functions (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__KeyboardEventUp(key, x, y);
    
    // Call this object's related event function
    KeyboardEventUp(key, x, y);
}

void GrfxObject::__SpecialKeyboardEvent(int key, int x, int y)
{
    // Call all children's related event functions (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__SpecialKeyboardEvent(key, x, y);
    
    // Call this object's related event function
    SpecialKeyboardEvent(key, x, y);
}

void GrfxObject::__MouseEvent(int button, int state, int x, int y)
{
    // Call all children's related event functions (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__MouseEvent(button, state, x, y);
    
    // Call this object's related event function
    MouseEvent(button, state, x, y);
}

void GrfxObject::__DragMouseEvent(int x, int y)
{
    // Call all children's related event functions (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__DragMouseEvent(x, y);
    
    // Call this object's related event function
    DragMouseEvent(x, y);
}

void GrfxObject::__PassiveMouseEvent(int x, int y)
{
    // Call all children's related event functions (Recursive)
    for(int i = 0; i < ChildObjects.GetSize(); i++)
        ChildObjects[i]->__PassiveMouseEvent(x, y);
    
    // Call this object's related event function
    PassiveMouseEvent(x, y);
}
