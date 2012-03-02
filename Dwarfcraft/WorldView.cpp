/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "WorldView.h"

WorldView::WorldView(WorldContainer* WorldData)
{
    // Save the world data
    this->WorldData = WorldData;
    
    // How many chunks are there for the x dimension
    ChunkCount = WorldData->GetWorldWidth() / WorldData->GetColumnWidth();
    
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

void WorldView::Render(Vector3<float> CameraPos, Vector3<float> CameraRight, int LayerCutoff)
{
    // Ignore y components
    CameraPos.y = 0;
    CameraRight.y = 0;
    
    // Helpful short-hand variable
    int ChunkWidth = WorldData->GetColumnWidth();
    
    // Max render distance
    const float MaxRenderDist = 10000.0f;
    
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
        if(ChunkGraphics->Planes == NULL)
            GenerateColumnVBO(ChunkX, ChunkZ);
        
        // For this chunk's height
        for(int i = 0; i < LayerCutoff + 1; i++)
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
            
            glPopMatrix();
            
//            // For each model
//            int ModelCount = Layer.Models.GetSize();
//            for(int j = 0; j < ModelCount; j++)
//            {
//                // Get model
//                glPushMatrix();
//                GameRender_Model Model = Layer.Models.Dequeue();
//                
//                // Render
//                glTranslatef(GlobalX + Model.x, GlobalY + Layer.DepthIndex, GlobalZ + Model.z);
//                
//                // Disabled models
//                Model.ModelData->Render();
//            }
            
            // Disabled for now
//            // Only render if this layer is the cutoff layer
//            if(Layer.DepthIndex == LayerCutoff)
//            {
//                /*** Hidden Layer ***/
//                
//                // Push local translation
//                glPushMatrix();
//                glTranslatef(GlobalX, GlobalY, GlobalZ);
//                Layer.HiddenGeometry->Render();
//                glPopMatrix();
//                
//                /*** Side Layer ***/
//                
//                // Push local translation
//                glPushMatrix();
//                glTranslatef(GlobalX, GlobalY, GlobalZ);
//                Layer.SideGeometry->Render();
//                glPopMatrix();
//            }
        }
    }
}

void WorldView::Update(float dT)
{
    // Nothing to do yet...
}

void WorldView::GenerateColumnVBO(int ChunkX, int ChunkZ)
{
    // Chunk we are working on and the world texture ID
    WorldView_Column* ChunkGraphics = &Chunks[ChunkZ * ChunkCount + ChunkX];
    GLuint WorldTextureID = dGetTerrainTextureID();
    
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
    
    // What is the global data origins?
    int OriginX = ChunkX * ColumnWidth;
    int OriginY = Y;
    int OriginZ = ChunkZ * ColumnWidth;
    
    // Is the source plane allocated and if not, what is the filling block type?
    bool IsFilled = !WorldData->GetChunk(ChunkX, ChunkZ)->Planes[OriginY].Allocated;
    dBlockType FillType;
    if(IsFilled)
        FillType = WorldData->GetChunk(ChunkX, ChunkZ)->Planes[OriginY].Data.PlaneType;
    
    // If the layer is just air, ignore
    if(IsFilled && FillType == dBlockType_Air)
        return false;
    
    // For each block
    const int y = Y;
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
        
//        // Case 2: Specialty 3D model (i.e. workbench)
//        else if(TargetBlock.GetType() == dBlockType_Mushroom)
//        {
//            // Allocate new model
//            GameRender_Model Model;
//            Model.x = x;
//            Model.z = z;
//            Model.ModelData = new VBuffer("MushroomModel.cfg");
//            
//            // Push to this layer's model list
//            LayerOut->Models.Enqueue(Model);
//        }
        
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
    
//    /*** Slice Detection ***/
//    
//    // If we have a block above (or after that), then mark black
//    for(int z = 0; z < WorldWidth; z++)
//    {
//        // Stack of x positions we should coalesce
//        Stack<int> BorderFaces;
//        
//        // We build x-length strips of black
//        // Note: we do [0...x] instead of [0...x) because we want to
//        // read off and parse the last few faces..
//        for(int x = 0; x <= WorldWidth; x++)
//        {
//            // If this is a black tile, we push to save...
//            if(x < WorldWidth && MainVolume->IsWithinWorld(x, LayerIndex + 1, z) && dIsSolid(MainVolume->GetBlock(x, LayerIndex + 1, z)))
//                BorderFaces.Push(x);
//            // Else, this is a solid tile and thus we are complete with our work
//            else if(!BorderFaces.IsEmpty())
//            {
//                // Get the geometry start and end
//                int EndX = BorderFaces.Pop();
//                int StartX = EndX;
//                while(!BorderFaces.IsEmpty())
//                    StartX = BorderFaces.Pop();
//                
//                // Place starting and ending vertices
//                // Ternary operator is to unduce overlap
//                static const float Overlap = -0.001f;
//                for(int i = 0; i < 2; i++)
//                    AddVertex(LayerOut->HiddenGeometry, Vector3<float>(StartX, LayerIndex, z + ((i == 0) ? Overlap : -Overlap)) + GameRender_FaceQuads[0][i], GameRender_Normals[0], i, dBlockType_Border);
//                for(int i = 2; i < 4; i++)
//                    AddVertex(LayerOut->HiddenGeometry, Vector3<float>(EndX, LayerIndex, z + ((i == 3) ? Overlap : -Overlap)) + GameRender_FaceQuads[0][i], GameRender_Normals[0], i, dBlockType_Border);
//            }
//        }
//    }
//    
//    /*** Side Removal ***/
//    
//    // Front side (0, ...)
//    for(int z = 0; z < WorldWidth; z++)
//    {
//        // If anything non-air, place the hidden surface
//        const int x = 0;
//        if(dIsSolid(MainVolume->GetBlock(x, LayerIndex, z).GetType()))
//        {
//            for(int i = 0; i < 4; i++)
//                AddVertex(LayerOut->SideGeometry, Vector3<float>(x, LayerIndex, z) + GameRender_FaceQuads[2][i], GameRender_Normals[2], i, dBlockType_Dirt);
//        }
//    }
//    
//    // Back side (WorldWidth - 1, ...)
//    for(int z = 0; z < WorldWidth; z++)
//    {
//        // If anything non-air, place the hidden surface
//        const int x = WorldWidth - 1;
//        if(dIsSolid(MainVolume->GetBlock(x, LayerIndex, z)))
//        {
//            for(int i = 0; i < 4; i++)
//                AddVertex(LayerOut->SideGeometry, Vector3<float>(x, LayerIndex, z) + GameRender_FaceQuads[1][i], GameRender_Normals[1], i, dBlockType_Dirt);
//        }
//    }
//    
//    // Left side (..., 0)
//    for(int x = 0; x < WorldWidth; x++)
//    {
//        // If anything non-air, place the hidden surface
//        const int z = 0;
//        if(dIsSolid(MainVolume->GetBlock(x, LayerIndex, z)))
//        {
//            for(int i = 0; i < 4; i++)
//                AddVertex(LayerOut->SideGeometry, Vector3<float>(x, LayerIndex, z) + GameRender_FaceQuads[3][i], GameRender_Normals[3], i, dBlockType_Dirt);
//        }
//    }
//    
//    // Left side (..., WorldWidth - 1)
//    for(int x = 0; x < WorldWidth; x++)
//    {
//        // If anything non-air, place the hidden surface
//        const int z = WorldWidth - 1;
//        if(dIsSolid(MainVolume->GetBlock(x, LayerIndex, z)))
//        {
//            for(int i = 0; i < 4; i++)
//                AddVertex(LayerOut->SideGeometry, Vector3<float>(x, LayerIndex, z) + GameRender_FaceQuads[4][i], GameRender_Normals[4], i, dBlockType_Dirt);
//        }
//    }
    
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
    if(Block.GetType() == dBlockType_Border)
        LightFactor = 1.0f;
    
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