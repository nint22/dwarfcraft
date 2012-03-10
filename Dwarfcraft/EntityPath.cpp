/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "EntityPath.h"

// Comparison function looks at the second value of the tuple-pair
// Used for sorting in our A* algorithm; internal function
static bool compare_NodeType(EntityPath::NodeType A, EntityPath::NodeType B)
{
    return A.second < B.second;
}

EntityPath::EntityPath(WorldContainer* WorldData, Vector3<int> Source, Vector3<int> Sink)
{
    // Save all references
    this->WorldData = WorldData;
    this->Source = Source;
    this->Sink = Sink;
    
    // Allocate the mutex
    pthread_mutex_init(&PathComputed, NULL);
    IsComputed = false;
}

EntityPath::~EntityPath()
{
    // Release mutex
    pthread_mutex_destroy(&PathComputed);
}

void EntityPath::ComputePath()
{
    // Launch the thread
    pthread_create(&MainThread, NULL, ComputePath, (void*)this);
}

bool EntityPath::GetPath(Stack< Vector3<int> >* Path)
{
    // Get the current completion state
    bool Complete = false;
    
    pthread_mutex_lock(&PathComputed);
    Complete = IsComputed;
    pthread_mutex_unlock(&PathComputed);
    
    // Post path if done
    if(Complete)
        *Path = this->ComputedPath;
    return Complete;
}

void* EntityPath::ComputePath(void* Data)
{
    // Start the internal timer (measures real-time, not thread-time, elapsed)
    UtilHighresClock ThreadClock(true);
    float TotalTime = 0.0f;
    
    // Convert to self object
    EntityPath* self = (EntityPath*)Data;
    
    // Build a queue of all adjacent paths we can visit, as well as the current path
    // Pair / tuple is (Position, Street-Distance)
    // Note that if a node's distance is set to a INT_MAX value, then that node
    // has been visited
    std::list<NodeType> ToVisit;
    std::list<NodeType> Visited;
    std::list<ComesFromType> ComesFrom;
    
    // Push the initial source position
    int InitialDistance = self->GetDistance(self->Source, self->Sink);
    ToVisit.push_front(NodeType(self->Source, InitialDistance));
    
    // Keep searching...
    bool Solved = false;
    while(!Solved)
    {
        // Check the total time for the hard limit
        ThreadClock.Stop();
        TotalTime += ThreadClock.GetTime();
        ThreadClock.Start();
        
        //if(TotalTime > EntityPath_MaxThreadTime)
        //    break;
        
        // If no nodes left, we were unable to find a solution
        if(ToVisit.size() <= 0)
            break;
        
        // Grab the highest-ranked node (already sorted) and mark as visited
        NodeType Node = ToVisit.front();
        ToVisit.pop_front();
        Visited.push_front(Node);
        
        // Push all adjacent positions of this new position
        // Notes: computes distance internally
        self->AddAdjacent(Node.first, &ComesFrom, &ToVisit, &Visited);
        
        // Sort based on the distance function; from least to greatest
        ToVisit.sort(compare_NodeType);
        
        // What we want to visit next
        NodeType NextPos = ToVisit.front();
        if(NextPos.second <= 0)
            Solved = true;
    }
    
    // Our final path
    Stack< Vector3<int> > Path;
    Path.Push(self->Sink);
    
    // If we found something...
    if(Solved)
    {
        // From the sink, go to the source (backtrace path)
        while(Path.Peek() != self->Source)
        {
            // What comes from the current path's node?
            bool FoundSource = false;
            std::list<ComesFromType>::iterator Node;
            for(Node = ComesFrom.begin(); Node != ComesFrom.end() && !FoundSource; Node++)
            {
                // Did we find a match?
                if(Node->first == Path.Peek())
                {
                    FoundSource = true;
                    Path.Push(Node->second);
                }
            }
            
            // If we never find a source, there is a consistency issue, fail out
            UtilAssert(FoundSource, "Internal consistency failure with path planning");
        }
    }
    
    // Mutex lock, post data, move on
    pthread_mutex_lock(&self->PathComputed);
    self->IsComputed = true;
    self->ComputedPath = Path;
    pthread_mutex_unlock(&self->PathComputed);
    
    // Done!
    return NULL;
}

int EntityPath::GetDistance(Vector3<int> A, Vector3<int> B)
{
    // Street-distance
    return abs(A.x - B.x) + abs(A.y - B.y) + abs(A.z - B.z);
}

bool EntityPath::IsUnique(Vector3<int> Position, std::list<NodeType>* ToVisit, std::list<NodeType>* Visited)
{
    // Does this position exist in either lists?
    std::list<NodeType>::iterator NodeTypeIt;
    
    for(NodeTypeIt = ToVisit->begin(); NodeTypeIt != ToVisit->end(); NodeTypeIt++)
    {
        if((*NodeTypeIt).first == Position)
            return false;
    }
    
    for(NodeTypeIt = Visited->begin(); NodeTypeIt != Visited->end(); NodeTypeIt++)
    {
        if((*NodeTypeIt).first == Position)
            return false;
    }
    
    // Never found, thus is unique
    return true;
}

int EntityPath::AddAdjacent(Vector3<int> Position, std::list<ComesFromType>* ComesFrom, std::list<NodeType>* ToVisit, std::list<NodeType>* Visited)
{
    // Entities can only go foward, left, right, backwards if the
    // height is the same, -0.5 or +0.5 (0.5 means half step)
    
    // All four positions
    static Vector3<int> Offsets[4] = {
        Vector3<int>(1, 0, 0),
        Vector3<int>(-1, 0, 0),
        Vector3<int>(0, 0, 1),
        Vector3<int>(0, 0, -1),
    };
    
    // For each position
    int NodeCount = 0;
    for(int i = 0; i < 4; i++)
    {
        // Create the offset position
        Vector3<int> Adjacent = Position + Offsets[i];
        
        // We will only look at the block below (has to be half step) or the block
        // ahead (which can either be empty or half) or the block above (on half block, moving to full)
        for(int j = -1; j <= 1; j++)
        {
            // Logic above: checking for block existance
            if(!WorldData->IsWithinWorld(Adjacent.x, Adjacent.y + j - 1, Adjacent.z))
                continue;
            if(!WorldData->IsWithinWorld(Adjacent.x, Adjacent.y + j, Adjacent.z))
                continue;
            if(!WorldData->IsWithinWorld(Adjacent.x, Adjacent.y + j + 1, Adjacent.z))
                continue;
            
            // Get the block spaces that we want to move into (or above, if half step)
            dBlock BelowTargetSpace = WorldData->GetBlock(Adjacent.x, Adjacent.y + j - 1, Adjacent.z);
            dBlock TargetSpace = WorldData->GetBlock(Adjacent.x, Adjacent.y + j, Adjacent.z);
            dBlock AboveTargetSpace = WorldData->GetBlock(Adjacent.x, Adjacent.y + j + 1, Adjacent.z);
            
            // Boolean flag set to true if target space is valid to move into; default to no-valid pos
            bool TargetValid = false;
            
            // If we are currently in full block (just air), apply different rules for transition
            // Note: We can only check if we are transitioning to the same level or below)
            if(WorldData->GetBlock(Position).IsWhole())
            {
                // If we are looking below, only half block acceptable
                if(j == -1 && dIsSolid(TargetSpace) && !TargetSpace.IsWhole() && AboveTargetSpace.GetType() == dBlockType_Air)
                    TargetValid = true;
                // If we are looking adjacent, only below full and target is empty air
                else if(j == 0 && dIsSolid(BelowTargetSpace) && BelowTargetSpace.IsWhole() && TargetSpace.GetType() == dBlockType_Air)
                    TargetValid = true;
                // If we are looking adjacent, adjacent can be a half block if top is air
                else if(j == 0 && dIsSolid(TargetSpace) && !TargetSpace.IsWhole() && AboveTargetSpace.GetType() == dBlockType_Air)
                    TargetValid = true;
            }
            // Start on a half-block
            else
            {
                // We can go adjacent if it is empty and below is full solid
                if(j == 0 && dIsSolid(BelowTargetSpace) && BelowTargetSpace.IsWhole() && TargetSpace.GetType() == dBlockType_Air)
                    TargetValid = true;
                // We can go adjacent if it is a step
                else if(j == 0 && dIsSolid(BelowTargetSpace) && BelowTargetSpace.IsWhole() && dIsSolid(TargetSpace) && !TargetSpace.IsWhole())
                    TargetValid = true;
                // We can go up if adjacent is a full solid
                else if(j == 1 && dIsSolid(BelowTargetSpace) && BelowTargetSpace.IsWhole() && TargetSpace.GetType() == dBlockType_Air)
                    TargetValid = true;
            }
            
            // Correct next position; if we are above 
            Vector3<int> NextPos(Adjacent.x, Adjacent.y + j, Adjacent.z);
            
            // If the target position we want to move into is valid and unique, then push
            if(TargetValid && IsUnique(NextPos, ToVisit, Visited))
            {
                // Added new node to our visiting list
                NodeCount++;
                ToVisit->push_back(NodeType(NextPos, GetDistance(NextPos, Sink)));
                
                // Add our node to our "comes from" list to build the path later on
                ComesFrom->push_back(ComesFromType(NextPos, Position));
                
                // Done searching valid paths in this column; move on to next adjacent
                break;
            }
        }
        
        // End of adjacency check
    }
    
    // All done and checked
    return NodeCount;
}
