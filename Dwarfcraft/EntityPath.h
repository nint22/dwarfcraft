/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: EntityPath.h/cpp
 Desc: Given a source and sink, applies a custom A* path-planning
 search function. In total it takes O(n^2) time; n being the number
 of traversable tiles.
 
 To use, you must pass the world geometry, source, sink, and then
 called the "compute" function. This function is threaded and thus
 needs to be queried over time for completion. The thread will
 keep running until either a path is found, or a hard limit of
 elapsed time is reached. Current hard-limit is 5 seconds.
 
 This path-planner also assumes that the entity can only move
 foward / back / left / right and up / down at half-step distances.
 
 Note: all paths, source, and sinks blocks are ALL through space,
 meaning it is a series of blocks that are above solid blocks (or
 in the case of half-blocks, within such blocks).
 
 Note: It is still critically important to look at all target
 blocks during run-time so that if the world is manipulated after
 path generation, the dwarves react correctly (i.e. computer new path,
 give up, etc..)
 
***************************************************************/

#ifndef __ENTITYPATH_H__
#define __ENTITYPATH_H__

#include <list>
#include <utility>
#include <limits.h>
#include "Queue.h"
#include "Stack.h"
#include "Vector3.h"
#include "WorldContainer.h"
#include <pthread.h>

// Hard time limit for the thread
static const float EntityPath_MaxThreadTime = 8.0f;

class EntityPath
{
public:
    
    // Standard constructor and destructor
    EntityPath(WorldContainer* WorldData, Vector3<int> Source, Vector3<int> Sink);
    ~EntityPath();
    
    // Compute a path; launches a thread that will give up after a hard-limit of time
    void ComputePath();
    
    // Retrieve the currently computed path; the calling function must compute the path first
    // Returns true when the thread is done; Posts the path data into the given buffer, else
    // returns an empty list.
    bool GetPath(Stack< Vector3<int> >* Path, bool* IsSolved);
    
    // Define the type that is associated with a node
    // It is both a position and a value; used so we can sort more easily
    // Note that if the second (distance) is set to a INT_MAX value if it was visited
    typedef std::pair< Vector3<int>, int > NodeType;
    
    // Define a "comes from" relationship to back-trace the correct path after a path is found
    // First is "this node", secon is "comes from"
    typedef std::pair< Vector3<int>, Vector3<int> > ComesFromType;
    
private:
    
    // Internal, threaded, compute-path function
    static void* ComputePath(void* Data);
    
    // Returns the street-distance which is the summation of component-deltas
    int GetDistance(Vector3<int> A, Vector3<int> B);
    
    // Returns true ig the given position doesn't exist in either lists
    bool IsUnique(Vector3<int> Position, std::list<NodeType>* ToVisit, std::list<NodeType>* Visited);
    
    // Add all valid adjacent positions into the given queue
    // May or may not add new positions, based on accessibility
    // Returns the number of nodes we are adding to the new positions list
    int AddAdjacent(Vector3<int> Position, std::list<ComesFromType>* ComesFrom, std::list<NodeType>* ToVisit, std::list<NodeType>* Visited);
    
    // World data handle
    WorldContainer* WorldData;
    
    // Source (origin) and sink (target)
    Vector3<int> Source, Sink;
    
    // Lock associated with the result boolean
    pthread_mutex_t PathComputed;
    bool IsComputed;
    Stack< Vector3<int> > ComputedPath;
    bool SolvedPath;
    
    // Main thread handle
    pthread_t MainThread;
};

#endif
