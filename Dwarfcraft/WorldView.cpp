/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "WorldView.h"

WorldView::WorldView(WorldContainer* WorldData, VolumeView* Designations, ItemsView* Items, StructsView* Structs, Entities* EntitiesList)
{
    // Save the world data and all other renderables
    this->WorldData = WorldData;
    
    // Save all the references, but only used for rendering data
    this->Designations = Designations;
    this->Items = Items;
    this->Structs = Structs;
    this->EntitiesList = EntitiesList;
    
    // How many chunks are there for the x dimension
    ChunkCount = WorldData->GetWorldWidth() / WorldData->GetColumnWidth();
    
    // Get the max render distance
    int ViewDist;
    GetUserSetting("General", "ViewDistance", &ViewDist, 10000);
    MaxRenderDist = ViewDist;
    
    // Allocate the chunks
    Chunks = new WorldView_Column[ChunkCount * ChunkCount];
    for(int i = 0; i < ChunkCount * ChunkCount; i++)
        Chunks[i].Planes = NULL;
}

WorldView::~WorldView()
{
    // Release the VBOs
    ClearVBO();
    
    // Release the graphical list itself
    delete[] Chunks;
}

void WorldView::Render(Vector3<float> CameraPos, Vector3<float> CameraRight, int LayerCutoff, float CameraAngle)
{
    // Ignore y components
    CameraPos.y = 0;
    CameraRight.y = 0;
    
    // Helpful short-hand variable
    int ChunkWidth = WorldData->GetColumnWidth();
    
    // For each chunk...
    for(int ChunkZ = 0; ChunkZ < ChunkCount; ChunkZ++)
    for(int ChunkX = 0; ChunkX < ChunkCount; ChunkX++)
    {
        // What is the vector from our camera to the chunk (global pos)
        // Note we are measuring from the middle of the chunk
        Vector3<float> ChunkVector = Vector3<float>(ChunkX * ChunkWidth + ChunkWidth / 2, 0, ChunkZ * ChunkWidth + ChunkWidth / 2) - CameraPos;
        
        // What is the distance? (Don't square it)
        if(ChunkVector.x * ChunkVector.x + ChunkVector.z * ChunkVector.z > MaxRenderDist)
            continue;
        
        // Cross it, only render positive results
        float Magnitude = Vector3Cross(CameraRight, ChunkVector).y;
        if(Magnitude < 0.0f)
            continue;
        
        // Get the chunk graphical data we are working on
        //WorldContainer_Column* ChunkData = WorldData->GetChunk(ChunkX, ChunkZ);
        WorldView_Column* ChunkGraphics = &Chunks[ChunkZ * ChunkCount + ChunkX];
        
        // If this chunk is not yet built, build the chunk
        if(ChunkGraphics->Planes == NULL || WorldData->GetChunk(ChunkX, ChunkZ)->NeedsUpdate)
        {
            GenerateColumnVBO(ChunkX, ChunkZ);
            WorldData->GetChunk(ChunkX, ChunkZ)->NeedsUpdate = false;
        }
        
        // For this chunk's height
        for(int i = 0; i <= LayerCutoff; i++)
        {
            // Get the layer and ignore if no geometry
            WorldView_Plane& Plane = ChunkGraphics->Planes[i];
            if(Plane.WorldGeometry == NULL)
                continue;
            
            /*** Regular Layers ***/
            
            // Push local translation
            glPushMatrix();
            
            // Only render if there is anything to render
            if(Plane.WorldGeometry != NULL)
                Plane.WorldGeometry->Render();
            
            // Render the hidden geometry
            if(i == LayerCutoff && Plane.HiddenGeometry != NULL)
                Plane.HiddenGeometry->Render();
            
            // Render the side geometry
            if(i == LayerCutoff && Plane.SideGeometry != NULL)
                Plane.SideGeometry->Render();
            
            // Render all models
            for(int ModelIndex = 0; ModelIndex < Plane.Models.GetSize(); ModelIndex++)
            {
                glPushMatrix();
                    Vector3<int> ModelPos = Plane.Models[ModelIndex].Position;
                    glTranslatef(ModelPos.x, ModelPos.y, ModelPos.z);
                    Plane.Models[ModelIndex].ModelData->Render();
                glPopMatrix();
            }
            
            // Done rendering this layer
            glPopMatrix();
        }
    }
    
    // Render all other entities
    Items->Render(LayerCutoff, CameraAngle);
    Designations->Render(LayerCutoff);
    Structs->Render();
    EntitiesList->Render(LayerCutoff, CameraAngle);
}

void WorldView::Update(float dT)
{
    // Update all sub-renderables
    Items->Update(dT);
    Designations->Update(dT);
    Structs->Update(dT);
    EntitiesList->Update(dT);
}

void WorldView::GenerateColumnVBO(int ChunkX, int ChunkZ)
{
    // Chunk we are working on and the world texture ID
    WorldView_Column* ChunkGraphics = &Chunks[ChunkZ * ChunkCount + ChunkX];
    GLuint WorldTextureID = dGetTerrainTextureID();
    
    // Release if already allocated
    if(ChunkGraphics->Planes != NULL)
    {
        for(int j = 0; j < WorldData->GetWorldHeight(); j++)
        {
            // Release all VBO data
            if(ChunkGraphics->Planes[j].WorldGeometry != NULL)
                ChunkGraphics->Planes[j].WorldGeometry->Clear();
            
            if(ChunkGraphics->Planes[j].HiddenGeometry != NULL)
                ChunkGraphics->Planes[j].HiddenGeometry->Clear();
            
            if(ChunkGraphics->Planes[j].SideGeometry != NULL)
                ChunkGraphics->Planes[j].SideGeometry->Clear();
            
            // Release all models
            for(int ModelIndex = 0; ModelIndex < ChunkGraphics->Planes[j].Models.GetSize(); ModelIndex++)
                delete ChunkGraphics->Planes[j].Models[ModelIndex].ModelData; // Internally releases VBO
        }
    }
    
    // Allocate all the layers (but default to NULL)
    ChunkGraphics->Planes = new WorldView_Plane[WorldData->GetWorldHeight()];
    for(int j = 0; j < WorldData->GetWorldHeight(); j++)
    {
        ChunkGraphics->Planes[j].WorldGeometry = NULL;
        ChunkGraphics->Planes[j].HiddenGeometry = NULL;
        ChunkGraphics->Planes[j].SideGeometry = NULL;
    }
    
    // For each layer, generate the VBO (game geometry and hidden volume)
    // Note: we are going from bottom (0) to top (depth - 1)
    for(int i = 0; i < WorldData->GetWorldHeight(); i++)
    {
        // Prepare a layer buffer to work on
        WorldView_Plane& Layer = ChunkGraphics->Planes[i];
        
        // Allocate geometry buffers (VBO-baseD)
        Layer.WorldGeometry = new VBuffer(GL_QUADS, WorldTextureID);
        Layer.HiddenGeometry = new VBuffer(GL_QUADS, WorldTextureID);
        Layer.SideGeometry = new VBuffer(GL_QUADS, WorldTextureID);
        
        // Generate, but release if 
        if(!GenerateLayerVBO(ChunkX, i, ChunkZ, &Layer))
        {
            // Release and null
            delete Layer.WorldGeometry;
            delete Layer.HiddenGeometry;
            delete Layer.SideGeometry;
            
            Layer.WorldGeometry = NULL;
            Layer.HiddenGeometry = NULL;
            Layer.SideGeometry = NULL;
        }
    }
}

bool WorldView::GenerateLayerVBO(int ChunkX, int Y, int ChunkZ, WorldView_Plane* Layer)
{
    /*** Generate Scene Geometry ***/
    
    // Short hand some data to easier / faster access
    const int ColumnWidth = WorldData->GetColumnWidth();
    const float StripOverlap = -0.001f;
    const int y = Y;
    
    // What is the global data origins?
    int OriginX = ChunkX * ColumnWidth;
    int OriginY = Y;
    int OriginZ = ChunkZ * ColumnWidth;
    
    // Is the source plane allocated and if not, what is the filling block type?
    bool IsFilled = !WorldData->GetChunk(ChunkX, ChunkZ)->Planes[OriginY].Allocated;
    dBlockType FillType;
    if(IsFilled)
        FillType = WorldData->GetChunk(ChunkX, ChunkZ)->Planes[OriginY].Data.PlaneType;
    
    /*** Cube Geometry ***/
    
    // If the layer is just air, ignore the cube geometry
    if(!IsFilled || FillType != dBlockType_Air)
    {
        // For each block
        for(int z = OriginZ; z < OriginZ + ColumnWidth; z++)
        for(int x = OriginX; x < OriginX + ColumnWidth; x++)
        {
            // Get block and ignore if air
            dBlock TargetBlock = WorldData->GetBlock(x, y, z);
            if(TargetBlock.GetType() == dBlockType_Air)
                continue;
            
            // Case 1: Regular block geometry
            if(!dHasSpecialGeometry(TargetBlock))
            {
                // For each face of the block, is it facing an air block or is it the top-most?
                // Six faces in total (Top, bottom, left, right, front, back)
                for(int OffsetIndex = 0; OffsetIndex < 5; OffsetIndex++)
                {
                    // Pull out the offset
                    Vector3<int> FaceOffset = GameRender_FaceOffsets[OffsetIndex];
                    
                    // Ignore if out of bounds
                    if(!WorldData->IsWithinWorld(x + FaceOffset.x, y + FaceOffset.y, z + FaceOffset.z))
                        continue;
                    
                    // Get block (we know it is in the world for sure)
                    dBlock AdjacentBlock = WorldData->GetBlock(x + FaceOffset.x, y + FaceOffset.y, z + FaceOffset.z);
                    
                    // Only render if next to a valid visible block
                    if(!dAdjacentCheck(TargetBlock, AdjacentBlock))
                        continue;
                    
                    // If we are checking the above-block, hide if above is a half-block (dAdjacentCheck can't do it...)
                    if(OffsetIndex == 0 && !AdjacentBlock.IsWhole())
                        continue;
                    
                    // If we are a whole block next to a half block, only render the top half of the vertices
                    if(OffsetIndex > 0 && TargetBlock.IsWhole() && !AdjacentBlock.IsWhole())
                    {
                        // Top geometry
                        for(int i = 0; i < 2; i++)
                            AddVertex(Layer->WorldGeometry, Vector3<float>(x, y, z) + WorldView_FaceQuads[OffsetIndex][i], WorldView_Normals[OffsetIndex], i, TargetBlock);
                        for(int i = 2; i < 4; i++)
                            AddVertex(Layer->WorldGeometry, Vector3<float>(x, y + 0.5f, z) + WorldView_FaceQuads[OffsetIndex][i], WorldView_Normals[OffsetIndex], i, TargetBlock);
                    }
                    // Normal geometry
                    else
                    {
                        // If this is a face we should render, push geometry into the queue
                        for(int i = 0; i < 4; i++)
                            AddVertex(Layer->WorldGeometry, Vector3<float>(x, y, z) + WorldView_FaceQuads[OffsetIndex][i], WorldView_Normals[OffsetIndex], i, TargetBlock);
                    }
                }
            }
            
            // Case 2: Specialty 3D model (i.e. workbenches, mushrooms, etc.)
            else if(TargetBlock.GetType() == dBlockType_Mushroom || TargetBlock.GetType() == dBlockType_Torch) // TORHC IS WORKBENCH DURING MODEL TESTING
            {
                // Allocate new model
                WorldView_Model Model;
                Model.Position = Vector3<int>(x, y, z);
                Model.Facing = dFacing_North;
                
                // Load the model
                Model.ModelData = new VBuffer("WorkBenchModel.cfg");
                
                // Push to this layer's model list
                int ModelCount = Layer->Models.GetSize();
                Layer->Models.Resize(ModelCount + 1);
                Layer->Models[ModelCount] = Model;
            }
                
            // Case 3: Generic 3D model (x-shape, for bushes, etc.)
            else
            {
                // Render only the sides, not the top
                for(int OffsetIndex = 1; OffsetIndex < 5; OffsetIndex++)
                {
                    for(int i = 0; i < 4; i++)
                        AddVertex(Layer->WorldGeometry, Vector3<float>(x, y, z) + WorldView_FaceQuads[OffsetIndex][i], WorldView_Normals[OffsetIndex], i, TargetBlock);
                }
            }
            
            // Done with this block
        }
    }
    
    /*** Slice Detection ***/
    
    // If we have a block above (or after that), then mark black
    for(int z = OriginZ; z < OriginZ + ColumnWidth; z++)
    {
        // Stack of x positions we should coalesce
        Stack<int> BorderFaces;
        
        // Build x-length strips of black
        for(int x = OriginX; x <= OriginX + ColumnWidth; x++)
        {
            // Make sure we are not at the last block, and within the world, to do an occlusion test
            if(x != (OriginX + ColumnWidth) &&
               WorldData->IsWithinWorld(x, y + 1, z) && dIsSolid(WorldData->GetBlock(x, y + 1, z)) &&
               WorldData->IsWithinWorld(x, y, z) && WorldData->GetBlock(x, y, z).GetType() != dBlockType_Air && WorldData->GetBlock(x, y, z).IsWhole())
            {
                BorderFaces.Push(x);
            }
            // Else, this is a solid tile and thus we are complete with this strip and start again
            else if(!BorderFaces.IsEmpty())
            {
                // Get the geometry start and end
                int EndX = BorderFaces.Pop();
                int StartX = EndX;
                while(!BorderFaces.IsEmpty())
                    StartX = BorderFaces.Pop();
                
                // Place starting and ending vertices
                // Ternary operator is to unduce overlap
                for(int i = 0; i < 2; i++)
                    AddVertex(Layer->HiddenGeometry, Vector3<float>(StartX, y, z + ((i == 0) ? StripOverlap : -StripOverlap)) + WorldView_FaceQuads[0][i]);
                for(int i = 2; i < 4; i++)
                    AddVertex(Layer->HiddenGeometry, Vector3<float>(EndX, y, z + ((i == 3) ? StripOverlap : -StripOverlap)) + WorldView_FaceQuads[0][i]);
            }
        }
    }
    
    /*** World-Bounds Occlusion ***/
    
    // If this column has a surface that touches a world edge
    for(int z = OriginZ; z < OriginZ + ColumnWidth; z++)
    for(int x = OriginX; x < OriginX + ColumnWidth; x++)
    {
        if(x == 0 || x == WorldData->GetWorldWidth() - 1 || z == 0 || z == WorldData->GetWorldWidth() - 1)
        {
            // A stack of blocks that must be occluded
            Stack<int> BorderFaces;
            
            // We are on a column that needs to have all sides occluded
            // d for depth
            for(int d = 0; d <= y; d++)
            {
                // If solid and not at the world end, hide
                bool IsSolid = dIsSolid(WorldData->GetBlock(x, d, z));
                if(IsSolid)
                    BorderFaces.Push(d);
                
                // Else, this is a gap, or the end of the column
                if((d == y || !IsSolid) && !BorderFaces.IsEmpty())
                {
                    // Get the strip start and end
                    int EndY = BorderFaces.Pop();
                    int StartY = EndY;
                    while(!BorderFaces.IsEmpty())
                        StartY = BorderFaces.Pop();
                    
                    // Which faces should we render here?
                    int FaceList[4] = {0, 0, 0, 0};
                    if(x == 0)
                        FaceList[0] = 2;
                    if(x == WorldData->GetWorldWidth() - 1)
                        FaceList[1] = 1;
                    if(z == 0)
                        FaceList[2] = 3;
                    if(z == WorldData->GetWorldWidth() - 1)
                        FaceList[3] = 4;
                    
                    // Save if the starting block is a half block or not
                    bool IsWhole = WorldData->GetBlock(x, EndY, z).IsWhole();
                    
                    // Push the strip geometry
                    for(int j = 0; j < 4; j++)
                    {
                        if(FaceList[j] != 0)
                        {
                            for(int i = 0; i < 2; i++) // High vertices
                                AddVertex(Layer->SideGeometry, Vector3<float>(x, EndY, z) + WorldView_FaceQuads[FaceList[j]][i] + (IsWhole ? Vector3<float>() : Vector3<float>(0, -.5f, 0)));
                            for(int i = 2; i < 4; i++) // Low vertices
                                AddVertex(Layer->SideGeometry, Vector3<float>(x, StartY, z) + WorldView_FaceQuads[FaceList[j]][i]);
                        }
                    }
                }
            }
            
            // Done with this column's occlusion
        }
    }
    
    /*** Finalize Geometry ***/
    
    // Generate geometry if there is data in either one
    if(!Layer->WorldGeometry->IsEmpty() || !Layer->HiddenGeometry->IsEmpty() || !Layer->SideGeometry->IsEmpty())
    {
        Layer->WorldGeometry->Generate();
        Layer->HiddenGeometry->Generate();
        Layer->SideGeometry->Generate();
        return true;
    }
    // Else, nothing to generate
    else
        return false;
}

void WorldView::AddVertex(VBuffer* Buffer, Vector3<float> Vertex, Vector3<float> Normal, int QuadCornerIndex, dBlock Block)
{
    // UV-texture based on the block type
    float x, y, width, height;
    dBlockFace Facing = dBlockFace_Top;
    
    if(Normal == WorldView_Normals[1])
        Facing = dBlockFace_Front;
    else if(Normal == WorldView_Normals[2])
        Facing = dBlockFace_Back;
    else if(Normal == WorldView_Normals[3])
        Facing = dBlockFace_Left;
    else if(Normal == WorldView_Normals[4])
        Facing = dBlockFace_Right;
    
    // If this is the top-face, set the level as needed or if this is any of
    // the uppser-sides, apply height diff. Note: the 1st and 2nd verticies
    // given will always be the upper-y
    if(!Block.IsWhole() && (Facing == dBlockFace_Top || QuadCornerIndex == 0 || QuadCornerIndex == 1))
        Vertex.y -= 0.5f;
    
    // Get the correct texture facing and details
    Vector3<float> TextureColor;
    int Rotations = 0;
    dGetBlockTexture(Block, Facing, &x, &y, &width, &height, &TextureColor, &Rotations);
    
    // Generic texture map; note that the z (third element) is ignored (UV, not UVW)
    Vector2<float> FaceTexture[4] =
    {
        Vector2<float>(x + width, y),
        Vector2<float>(x, y),
        Vector2<float>(x, y + height),
        Vector2<float>(x + width, y + height),
    };
    
    // How many rotations must we do?
    for(int i = 0; i < Rotations; i++)
    {
        Vector2<float> Temp = FaceTexture[0];
        FaceTexture[0] = FaceTexture[1];
        FaceTexture[1] = FaceTexture[2];
        FaceTexture[2] = FaceTexture[3];
        FaceTexture[3] = Temp;
    }
    
    // Compute the color as the dot product between the sun and the this normal
    // Note: the constants were just test-and-compile derived
    float LightRand = 0.12f * (float(rand()) / float(RAND_MAX));
    float LightDepth = 1.0f + 0.32f * (Vertex.y - float(WorldData->GetWorldHeight() - 16)) / 16.0f;
    float LightNormal = 0.2f * Vector3Dot(Vector3<float>(1, 2, 4), Normal);
    
    float LightFactor = fmin(1.0f, fmax(0.4f, LightRand + LightDepth + LightNormal));
    
    // Apply occlusion factor
    // Little hack: if it's a half block, we have to bump the vertex's y up a half
    if(Vertex.y - (int)Vertex.y > 0.0f)
        LightFactor *= GetAmbientOcclusion(Vector3ftoi(Vertex) + Vector3<int>(0, 1, 0));
    else
        LightFactor *= GetAmbientOcclusion(Vector3ftoi(Vertex));
    
    // Add vertices
    if(dHasSpecialGeometry(Block))
    {
        if(Facing == dBlockFace_Front)
            Vertex.x -= 0.5f;
        else if(Facing == dBlockFace_Back)
            Vertex.x += 0.5f;
        
        if(Facing == dBlockFace_Left)
            Vertex.z += 0.5f;
        else if(Facing == dBlockFace_Right)
            Vertex.z -= 0.5f;
    }
    
    // Add the full data set for the given vertex into the buffer
    Buffer->AddVertex(Vertex, TextureColor * LightFactor, FaceTexture[QuadCornerIndex]);
}

void WorldView::AddVertex(VBuffer* Buffer, Vector3<float> Vertex)
{
    // Black color, no UV texture
    Vector3<float> VertexColor(0, 0, 0);
    Buffer->AddVertex(Vertex, VertexColor, Vector2<float>());
}

void WorldView::ClearVBO()
{
    // For each column
    for(int i = 0; i < ChunkCount * ChunkCount; i++)
    {
        // For each plane
        for(int j = 0; j < WorldData->GetWorldHeight(); j++)
        {
            // Release the pointers
            delete Chunks[i].Planes[j].WorldGeometry;
            delete Chunks[i].Planes[j].HiddenGeometry;
            delete Chunks[i].Planes[j].SideGeometry;
            
            // Set to null
            Chunks[i].Planes[j].WorldGeometry = NULL;
            Chunks[i].Planes[j].HiddenGeometry = NULL;
            Chunks[i].Planes[j].SideGeometry = NULL;
        }
        
        // Release and set to null
        delete[] Chunks[i].Planes;
        Chunks[i].Planes = NULL;
    }
}

float WorldView::GetAmbientOcclusion(Vector3<int> Pos)
{
    // At first, we have full lights
    float Occlusion = 1.0f;
    
    // For each block around this vertex...
    for(int y = 0; y < 2; y++)
    for(int x = 0; x < 2; x++)
    for(int z = 0; z < 2; z++)
    {
        // Get the block offset position
        Vector3<int> BlockPos = Pos + Vector3<int>(x, y, z);
        if(WorldData->IsWithinWorld(BlockPos) && WorldData->GetBlock(BlockPos).GetType() != dBlockType_Air)
            Occlusion -= 1.0f / 8.0f; // 8 Blocks are being traversed
    }
    
    // Return the computed occlusion
    return Occlusion;
}
