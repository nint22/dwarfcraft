/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Designations.h/cpp
 Desc: Manages all game designations for the givn map. This
 is generally a container of all designations for a given world.
 
***************************************************************/

#ifndef __DESIGNATIONSVIEW_H__
#define __DESIGNATIONSVIEW_H__

#include "WorldContainer.h"
#include "Vector3.h"
#include "Queue.h"

// 16 total designations
static const int DesignationCount = 16;

// Enumeration of designations
enum DesignationType
{
    // Construction
    DesignationType_Mine = 0,
    DesignationType_Fill,
    DesignationType_Flood,
    
    // Storage
    DesignationType_Rubbish,
    DesignationType_Food,
    DesignationType_Crafted,
    DesignationType_RawResources, // Wood, stones, etc.
    DesignationType_Ingots,
    DesignationType_Grave,
    
    // Collection
    DesignationType_Farm,
    DesignationType_Wood,
    DesignationType_Forage,
    
    // Military flags
    DesignationType_Protect,
    DesignationType_Barracks,
    DesignationType_Hall,
    DesignationType_Armory,
};

// Human readable names
static const char DesignationNames[DesignationCount][32] =
{
    "Mine",
    "Fill",
    "Flood",
    "Rubbish",
    "Food",
    "Crafted",
    "RawResources",
    "Ingots",
    "Grave",
    "Farm",
    "Wood",
    "Forage",
    "Protect",
    "Barracks",
    "Hall",
    "Armory",
};

// A designation data type
struct Designation
{
    DesignationType Type;
    Vector3<int> Origin;
    Vector3<int> Volume;
};

// Returns the texture coordinates of a given designation; similar to the "dGetBlockTexture(...)" function
void dGetDesignationTexture(DesignationType Type, float* x, float* y, float* width, float* height);

class DesignationsView
{
public:
    
    // Standard constructor and destructor
    DesignationsView(WorldContainer* MainWorld);
    ~DesignationsView();
    
    // Add a new designation (takes any two points; will self-organize for origin and volume)
    void AddDesignation(DesignationType Type, Vector3<int> Origin, Vector3<int> Volume);
    
    // Remove a designation that contains the given point
    void RemoveDesignation(Vector3<int> Point);
    
    // Given a designation, find all accesable blocks to work on. A queue of positions pairs is returned. The first
    // element in the tuple is the block that can be manipulated in the designation volume, while the second value
    // is a (possibly) accesable block by the dwarf (position to be in, not on). An empty list may be returned as
    // well if nothing is accesable.
    Queue< std::pair< Vector3<int>, Vector3<int> > > FindDesignation(DesignationType Type, Vector3<int> Origin);
    
    // Get a copy (not reference) of all designations
    Queue< Designation > GetDesignations();
    
    // Posts two lists, each in parallel of the other, which represents the designation type and screen position
    void GetDesignationsStrings(List< DesignationType >** DesignationTypes, List< Vector2<int> >** ScreenPositions);
    
    /*** Core Render & Update ***/
    
    // Update and possible regenerate some VBOs
    void Update(float dT);
    
    // Render this Volume (does all translations internally)
    void Render(int LayerCutoff, bool Draw = true);
    
    // Need to explicitly set the window size over time
    void SetWindowSize(int Width, int Height);
    
private:
    
    // Take world positions and turn it into screen positions
    Vector2<int> WorldToScreen(Vector3<float> Position);
    
    // Window height
    int WindowHeight;
    
    // World cutoff
    int Cutoff;
    
    // List of all designations
    Queue< Designation > DesignationsQueue;
    
    // World handle
    WorldContainer* WorldData;
    
    // Designation data used for saving label positions, rendered in UserInterface
    List< DesignationType > SavedDesignationTypes;
    List< Vector2<int> > SavedScreenPositions;
    
};

#endif
