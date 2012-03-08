/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: WorldContainer.cpp
 Desc: A container for world geometry. The old data retainer of
 the world was an Octree, which allowed for large areas to not
 be allocated, but did have an overhead of O(log(n)) time.
 
 This new container is a mix of on-the-fly allocation and
 chunkify design: the world is made up of world columns, where
 each column is made up of individual floors. Each floor is made
 up of a chunk length and chunk width 2D plane of cube. Each cube
 is specificly a "dBlock", but a block can either be a full block
 or a half block.
 
 The look-up times for either the world chunks or levels is constant,
 and memory is optimized using two states of allocations per floor:
 
 1. Homogeneous allocation (all blocks are the same, thus nothing
    is allocated, just a flag is raised declaring what the entire
    chunk is)
 2. Heterogeneous (mix of blocks, full allocation done)
 
 Worlds are not continuous and do have limited boundaries.
 
 Upon world generation, the entire world has the minimal framework
 allocated (which is a 2D array of empty columns, each column being
 and array of empty pointers, all representing air). By placing
 a non-air block, that entire floor is allocated, though the rest
 of the original cubes are left to be air / empty.
 
 This structure uses OpenGL's left-handed coordinates: x and z
 go across the plane, while y traverses the volume's depth, y+
 going up.
 
***************************************************************/

// Inclusion guard
#ifndef __WORLDCONTAINER_H__
#define __WORLDCONTAINER_H__

#include "dBlocks.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Queue.h"
#include "Globals.h"

// Plane structure, representing a plane within a column
struct WorldContainer_Plane
{
    // If false, the volume is homogenous
    bool Allocated;
    
    // Unioned to save space, since the type is mutually exclusive
    union {
        dBlockType PlaneType;
        dBlock* PlaneData;
    } Data;
};

// Column structure
struct WorldContainer_Column
{
    // A list of plane pointers, defaultes to NULL
    // if a floor is not allocated. Indexed from 0 (bottom)
    // to n-1 (top)
    WorldContainer_Plane* Planes;
};

class WorldContainer
{
public:
    
    // Takes world width, depth, and column size. World is always a square, same as a column
    // The width must be divisible by column; i.e. width = 128, while column is 8
    WorldContainer(int Width, int Height, int ColumnWidth);
    ~WorldContainer();
    
    // Get world size
    int GetWorldWidth();
    
    // Get world depth
    int GetWorldHeight();
    
    // Get column size (width)
    int GetColumnWidth();
    
    // Returns true if within the world bounds
    bool IsWithinWorld(int x, int y, int z);
    bool IsWithinWorld(Vector3<int> Pos);
    bool IsWithinWorld(Vector3<float> Pos);
    
    // Access a block at the given location; no bounds-checking for speed bonus
    dBlock GetBlock(int x, int y, int z);
    dBlock GetBlock(Vector3<int> Pos);
    dBlock GetBlock(Vector3<float> Pos);
    
    // Sets a block at the given location; no bounds-checking for speed bonus
    void SetBlock(int x, int y, int z, dBlock Block);
    void SetBlock(Vector3<int> Pos, dBlock Block);
    void SetBlock(Vector3<float> Pos, dBlock Block);
    
    // Fill a given column's plane with one type of block
    // For performance gain, always use this when doing large volume fillings
    void FillChunk(int x, int y, int z, dBlockType BlockType);
    void FillChunk(Vector3<int> Pos, dBlockType BlockType);
    void FillChunk(Vector3<float> Pos, dBlockType BlockType);
    
    // Get a chunk at the given x, z location (returns the entire column)
    // Note: The given positions are CHUNK positions, not world positions
    WorldContainer_Column* GetChunk(int x, int z);
    WorldContainer_Column* GetChunk(Vector2<int> Pos);
    
    // Optimize the geometry; this may take time but will coalesce all data if possible
    // Warning: this is a slow function
    void OptimizeColumns();
    
    // Get the first non-air block from the top-towards-bottom in the given column
    int GetSurfaceDepth(int x, int z);
    
    // Get the first non-air block from the given layer y (inclusive)
    int GetSurfaceDepth(int x, int y, int z);
    
    // Intersect the world and return the first collision the given ray intersects
    // Cutoff layer is the index, from the bottom (0) of the last row we check
    bool IntersectWorld(Vector3<float> RayPos, Vector3<float> RayDir, int CutoffLayer, Vector3<int>* CollisionBox);
    
private:
    
    // World size properties (and subset)
    int WorldWidth, WorldHeight, ColumnWidth, ChunkCount;
    
    // List of all chunks (just 2D array of size [Width / ColumnWidth][Width / ColumnWidth])
    WorldContainer_Column* WorldChunks;
    
    // Max render distance
    float MaxRenderDist;
};

#endif
