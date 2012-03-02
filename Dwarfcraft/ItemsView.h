/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: ItemsView.h/cpp
 Desc: An item container and renderer. Renders / handles
 all world items (deriving from Items.png) which is defined in
 dBlocks.h This class gets called / is managed by the world view
 
***************************************************************/

#ifndef __ITEMSVIEW_H__
#define __ITEMSVIEW_H__

#include "GrfxObject.h"
#include "List.h"
#include "dBlocks.h"
#include "Vector3.h"

// Renderable item structure
struct ItemsView_Item // Renderable item
{
    // Base item
    dItem Item;
    
    // Current location and movement vector (when in "pop'ed" state)
    bool Settled; // If settled, no longer animating
    Vector3<float> Pos;
    Vector3<float> Vel;
    
    // Total time simulated
    float dT;
};

class ItemsView : public GrfxObject
{
public:
    
    // Standard constructor and desctructor
    ItemsView(GrfxObject* Parent, int WorldDepth);
    ~ItemsView();
    
    // Add an item to the world at the given position
    void AddItem(dItem Item, Vector3<int> Pos);
    
    // Remove an item from the world
    void RemoveItem(Vector3<int> Pos);
    
    // Needs to update; sets the camera angle
    void SetCameraAngle(float Theta);
    
    // Gets the camera angle
    float GetCameraAngle();
    
    // Needs to update; makes sure no above-surface items are rendered
    void SetLayerCutoff(int Cutoff);
    
    // Get the current cutoff
    int GetLayerCutoff();
    
protected:
    
    // Standard render and update functions from GrfxObject
    void Render();
    void Update(float dT);
    
private:
    
    // Render a billboard
    void RenderBillboard(Vector3<float> pos, float srcx, float srcy, float srcwidth, float srcheight, float doffset = 0.0f);
    
    // Render a circle beneath the item
    void RenderShadow(Vector3<float> pos, float radius);
    
    // World depth
    int WorldDepth;
    
    // List of items (a list per each level)
    List< ItemsView_Item >* ItemLevels;
    
    // Texture ID
    GLuint TextureID;
    
    // World camera angle
    float CameraTheta;
    
    // World cutoff
    int Cutoff;
};

#endif
