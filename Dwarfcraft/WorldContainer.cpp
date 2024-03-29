/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "WorldContainer.h"

WorldContainer::WorldContainer(int Width, int Height, int ColumnWidth)
{
    // Assert all valid
    UtilAssert(Width > 0 && Height > 0, "Given world width or depth are not positive values");
    UtilAssert(Width % ColumnWidth == 0 && ColumnWidth <= Width, "Given world width is not a multiple of the column width");
    
    // Save all given info
    WorldWidth = Width;
    WorldHeight = Height;
    this->ColumnWidth = ColumnWidth;
    
    // Allocate world column container
    ChunkCount = WorldWidth / ColumnWidth;
    WorldChunks = new WorldContainer_Column[ChunkCount * ChunkCount];
    
    // For each column, allocate with nothing inside
    for(int z = 0; z < ChunkCount; z++)
    for(int x = 0; x < ChunkCount; x++)
    {
        // Allocate all the planes, but leave unallocated
        WorldContainer_Plane* Levels = new WorldContainer_Plane[WorldHeight];
        WorldChunks[z * ChunkCount + x].Planes = Levels;
        WorldChunks[z * ChunkCount + x].NeedsUpdate = false;
        for(int y = 0; y < WorldHeight; y++)
        {
            Levels[y].Allocated = false;
            Levels[y].Data.PlaneType = dBlockType_Air;
        }
    }
    
    // Get camera render distance
    int ViewDist;
    GetUserSetting("General", "ViewDistance", &ViewDist, 10000);
    MaxRenderDist = ViewDist;
}

WorldContainer::~WorldContainer()
{
    // For each column, allocate with nothing inside
    for(int z = 0; z < ChunkCount; z++)
    for(int x = 0; x < ChunkCount; x++)
    {
        // For each level, release the internal allocation
        WorldContainer_Plane* Levels = WorldChunks[z * ChunkCount + x].Planes;
        for(int y = 0; y < WorldHeight; y++)
        {
            if(Levels[y].Allocated)
                delete[] Levels[y].Data.PlaneData;
        }
        
        // Delete the column
        delete[] Levels;
    }
    
    // Delete the world chunks list
    delete[] WorldChunks;
}

int WorldContainer::GetWorldWidth()
{
    return WorldWidth;
}

int WorldContainer::GetWorldHeight()
{
    return WorldHeight;
}

int WorldContainer::GetColumnWidth()
{
    return ColumnWidth;
}

bool WorldContainer::IsWithinWorld(int x, int y, int z)
{
    if(x >= 0 && x < WorldWidth && z >= 0 && z < WorldWidth && y >= 0 && y < WorldHeight)
        return true;
    else
        return false;
}

bool WorldContainer::IsWithinWorld(Vector3<int> Pos)
{
    return IsWithinWorld(Pos.x, Pos.y, Pos.z);
}

bool WorldContainer::IsWithinWorld(Vector3<float> Pos)
{
    return IsWithinWorld(Pos.x, Pos.y, Pos.z);
}

dBlock WorldContainer::GetBlock(int x, int y, int z)
{
    // Find the chunk
    int cx = x / ColumnWidth;
    int cz = z / ColumnWidth;
    int dx = x % ColumnWidth;
    int dz = z % ColumnWidth;
    
    // Target plane (ref variable)
    WorldContainer_Plane& Plane = WorldChunks[cz * ChunkCount + cx].Planes[y];
    
    // If allocated, return block, else, return the position
    if(Plane.Allocated)
        return Plane.Data.PlaneData[dz * ColumnWidth + dx];
    else
        return dBlock(Plane.Data.PlaneType);
}

dBlock WorldContainer::GetBlock(Vector3<int> Pos)
{
    return GetBlock(Pos.x, Pos.y, Pos.z);
}

dBlock WorldContainer::GetBlock(Vector3<float> Pos)
{
    return GetBlock(Pos.x, Pos.y, Pos.z);
}

void WorldContainer::SetBlock(int x, int y, int z, dBlock Block)
{
    // Find the chunk
    int cx = x / ColumnWidth;
    int cz = z / ColumnWidth;
    
    // Localize change
    int dx = x % ColumnWidth;
    int dz = z % ColumnWidth;
    
    // Target plane (ref variable)
    WorldChunks[cz * ChunkCount + cx].NeedsUpdate = true;
    WorldContainer_Plane& Plane = WorldChunks[cz * ChunkCount + cx].Planes[y];
    
    // If allocated, just assign block
    if(Plane.Allocated)
        Plane.Data.PlaneData[dz * ColumnWidth + dx] = Block;
    
    // Else, only allocate if it is a new type
    else if(Plane.Data.PlaneType != Block.GetType())
    {
        // Allocate and fill with air, then set the block
        Plane.Allocated = true;
        Plane.Data.PlaneData = new dBlock[ColumnWidth * ColumnWidth];
        for(int i = 0; i < ColumnWidth * ColumnWidth; i++)
            Plane.Data.PlaneData[i] = dBlock(dBlockType_Air);
        
        // Set the target block
        Plane.Data.PlaneData[dz * ColumnWidth + dx] = Block;
    }
    
    // Note: if the changed block is on a column bound, update the adjacent
    if(dx == 0 && cx > 0)
        WorldChunks[cz * ChunkCount + (cx - 1)].NeedsUpdate = true;
    if(dx == (ColumnWidth - 1) && cx < ChunkCount - 1)
        WorldChunks[cz * ChunkCount + (cx + 1)].NeedsUpdate = true;
    if(dz == 0 && cz > 0)
        WorldChunks[(cz - 1) * ChunkCount + cx].NeedsUpdate = true;
    if(dz == (ColumnWidth - 1) && cz < ChunkCount - 1)
        WorldChunks[(cz + 1) * ChunkCount + cx].NeedsUpdate = true;
}

void WorldContainer::SetBlock(Vector3<int> Pos, dBlock Block)
{
    SetBlock(Pos.x, Pos.y, Pos.z, Block);
}

void WorldContainer::SetBlock(Vector3<float> Pos, dBlock Block)
{
    SetBlock(Pos.x, Pos.y, Pos.z, Block);
}

void WorldContainer::FillChunk(int x, int y, int z, dBlockType BlockType)
{
    // Find the chunk
    int cx = x / ColumnWidth;
    int cz = z / ColumnWidth;
    
    // Target plane (ref variable)
    WorldChunks[cz * ChunkCount + cx].NeedsUpdate = true;
    WorldContainer_Plane& Plane = WorldChunks[cz * ChunkCount + cx].Planes[y];
    
    // Release if needed
    if(Plane.Allocated)
        delete[] Plane.Data.PlaneData;
    
    // Set the type and allocation flag
    Plane.Allocated = false;
    Plane.Data.PlaneType = BlockType;
}

void WorldContainer::FillChunk(Vector3<int> Pos, dBlockType BlockType)
{
    FillChunk(Pos.x, Pos.y, Pos.z, BlockType);
}

void WorldContainer::FillChunk(Vector3<float> Pos, dBlockType BlockType)
{
    FillChunk(Pos.x, Pos.y, Pos.z, BlockType);
}

WorldContainer_Column* WorldContainer::GetChunk(int x, int z)
{
    // Return the chunk
    return &WorldChunks[z * ChunkCount + x];
}

WorldContainer_Column* WorldContainer::GetChunk(Vector2<int> Pos)
{
    return GetChunk(Pos.x, Pos.y);
}

void WorldContainer::OptimizeColumns()
{
    // For each column, allocate with nothing inside
    for(int z = 0; z < ChunkCount; z++)
    for(int x = 0; x < ChunkCount; x++)
    {
        // For each level..
        WorldContainer_Plane* Levels = WorldChunks[z * ChunkCount + x].Planes;
        for(int y = 0; y < WorldHeight; y++)
        {
            // If allocated, see if possibly homogeneous
            if(Levels[y].Allocated)
            {
                // Do all other elements match?
                bool IsUniform = true;
                dBlockType BlockType = Levels[y].Data.PlaneData[0].GetType();
                for(int i = 0; i < WorldWidth * WorldWidth - 1 && !IsUniform; i++)
                {
                    // TODO: Check meta tag?
                    if(Levels[y].Data.PlaneData[i].GetType() == Levels[y].Data.PlaneData[i + 1].GetType())
                        IsUniform = false;
                }
                
                // If uniform, deallocate
                if(!IsUniform)
                {
                    Levels[y].Allocated = false;
                    delete[] Levels[y].Data.PlaneData;
                    Levels[y].Data.PlaneType = BlockType;
                }
            }
        }
    }
}

int WorldContainer::GetSurfaceDepth(int x, int z)
{
    return GetSurfaceDepth(x, WorldHeight - 1, z);
}

int WorldContainer::GetSurfaceDepth(int x, int y, int z)
{
    // From top to bottom
    for(int i = y; i >= 0; i--)
        if(GetBlock(x, i, z).GetType() != dBlockType_Air)
            return i;
    
    // Never found, just return the top-most block
    return y;
}

bool WorldContainer::IntersectWorld(Vector3<float> RayPos, Vector3<float> RayDir, int CutoffLayer, Vector3<int>* CollisionBox)
{
    // Queue of chunks that are intersected
    Queue< Vector3<int> > IntersectedChunks;
    
    // For each chunk
    for(int z = 0; z < ChunkCount; z++)
    for(int x = 0; x < ChunkCount; x++)
    {
        // Only test chunks that are close enough
        // What is the vector from our camera to the chunk (global pos)
        // Note we are measuring from the middle of the chunk
        Vector3<float> ChunkVector = Vector3<float>(x * ColumnWidth, 0, z * ColumnWidth) - RayPos;
        
        // What is the distance? (Don't square it)
        if(ChunkVector.x * ChunkVector.x + ChunkVector.z * ChunkVector.z > MaxRenderDist)
            continue;
        
        // Make some variables arrays for easy access (index maps to x,y,z)
        float _BoxPos[3] = { x * ColumnWidth, 0, z * ColumnWidth };
        float _RayPos[3] = { RayPos.x, RayPos.y, RayPos.z };
        float _RayDir[3] = { RayDir.x, RayDir.y, RayDir.z };
        
        // Defaulted to limits as an "invalid" flag
        float Near = -INFINITY;
        float Far = INFINITY;
        
        // Default to valid
        bool IsValid = true;
        
        // For each dimension's surface planes
        // For example x means the + and - surface parallel to the yz plane
        for(int i = 0; i < 3 && IsValid; i++)
        {
            // Calculate x-plane intersections
            // Terenary operator is to make clear that x and z are chunk sizes while y is world height
            float MinX = (_BoxPos[i] - _RayPos[i]) / _RayDir[i];
            float MaxX = ((_BoxPos[i] + ((i == 1) ? WorldHeight : ColumnWidth)) - _RayPos[i]) / _RayDir[i];
            
            // Swap min/max values
            if(MinX > MaxX)
            {
                float temp = MinX;
                MinX = MaxX;
                MaxX = temp;
            }
            
            // Bounds set
            if(MinX > Near)
                Near = MinX;
            if(MaxX < Far)
                Far = MaxX;
            
            // Box was missed
            if(Near > Far)
                IsValid = false;
            else if(Far < 0)
                IsValid = false;
        }
        
        // This is layer we are coliding with
        if(IsValid)
            IntersectedChunks.Enqueue(Vector3<int>(x, 0, z));
    }
    
    // From top (cutoff) to bottom
    for(int y = CutoffLayer; y >= 0; y--)
    {
        // For each chunk
        const int ChunkCount = IntersectedChunks.GetSize();
        for(int c = 0; c < ChunkCount; c++)
        {
            // Get an intersected level
            Vector3<int> ChunkPos = IntersectedChunks.Dequeue();
            
            // The start of the global chunk position
            const int gx = ChunkPos.x * ColumnWidth;
            const int gy = 0; // Just for consistencies sake
            const int gz = ChunkPos.z * ColumnWidth;
            
            // For each cube in this chunk's layer
            for(int x = 0; x < ColumnWidth; x++)
            for(int z = 0; z < ColumnWidth; z++)
            {
                // Make some variables arrays for easy access (index maps to x,y,z)
                float _BoxPos[3] = { gx + x, gy + y, gz + z };
                float _RayPos[3] = { RayPos.x, RayPos.y, RayPos.z };
                float _RayDir[3] = { RayDir.x, RayDir.y, RayDir.z };
                
                // Are we in this chunk?
                float Near = -INFINITY;
                float Far = INFINITY;
                
                // Default to valid
                bool IsValid = true;
                
                // For each dimension's surface planes
                // For example x means the + and - surface parallel to the yz plane
                for(int i = 0; i < 3 && IsValid; i++)
                {
                    // Calculate x-plane intersections (cubes are always 1^3)
                    float MinX = (_BoxPos[i] - _RayPos[i]) / _RayDir[i];
                    float MaxX = ((_BoxPos[i] + 1) - _RayPos[i]) / _RayDir[i];
                    
                    // Swap min/max values
                    if(MinX > MaxX)
                    {
                        float temp = MinX;
                        MinX = MaxX;
                        MaxX = temp;
                    }
                    
                    // Bounds set
                    if(MinX > Near)
                        Near = MinX;
                    if(MaxX < Far)
                        Far = MaxX;
                    
                    // Box was missed
                    if(Near > Far)
                        IsValid = false;
                    else if(Far < 0)
                        IsValid = false;
                }
                
                // This is layer we are coliding with and it isn't air
                if(IsValid && GetBlock(gx + x, gy + y, gz + z).GetType() != dBlockType_Air)
                {
                    // Post and stop
                    *CollisionBox = Vector3<int>(gx + x, gy + y, gz + z);
                    return true;
                }
            }
            
            // Enqueue the chunk we are testing back
            IntersectedChunks.Enqueue(ChunkPos);
        }
    }
    
    // Else, no collisions ever found
    return false;
}
