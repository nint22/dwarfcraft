/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "StructsView.h"

StructsView::StructsView(WorldContainer* WorldData)
{
    // Save world data
    this->WorldData = WorldData;
    
    // Allocate an array of item levels
    Structs = new List< StructsView_Struct >[WorldDepth];
    this->WorldDepth = WorldDepth;
}

StructsView::~StructsView()
{
    // For each layer
    for(int LayerIndex = 0; LayerIndex < WorldDepth; LayerIndex++)
    {
        // For each strut
        for(int i = 0; i < Structs[LayerIndex].GetSize(); i++)
            delete Structs[LayerIndex][i].ModelData;
    }
    
    // Release the array list
    delete[] Structs;
}

void StructsView::AddStruct(dBlock Struct, Vector3<int> Pos)
{
    /*
    // Grow list size, add to end
    int OriginalLength = Structs[Pos.y].GetSize();
    Structs[Pos.y].Resize(OriginalLength + 1);
    
    // Create the structure itself
    StructsView_Struct NewStruct;
    NewStruct.Struct = Struct;
    NewStruct.Pos = Pos;
    NewStruct.dT = float(rand()) / float(RAND_MAX); // Randomize the initial time
    
    // Given the struct type, allocate the model data
    if(Struct.GetType() == dBlockType_Caravan)
        NewStruct.ModelData = new VBuffer("CaravanModel.cfg");
    
    // Else, never found, fail out
    else
        UtilAssert(false, "Unable to match a model configuration file for the given block: %d\n", Struct.GetType());
    
    // Add to the structs list
    Structs[Pos.y][OriginalLength] = NewStruct;
    */
}

void StructsView::RemoveStruct(Vector3<int> Pos)
{
    // Seek the list for items at the position
    int OriginalLength = Structs[Pos.y].GetSize();
    for(int i = 0; i < OriginalLength; i++)
    {
        // If found, shift over from right, then resize
        Vector3<int> StructPos = Structs[Pos.y][i].Pos;
        if(StructPos == Pos)
        {
            // Shift over from right
            for(int j = i; j < OriginalLength - 1; j++)
                Structs[Pos.y][j] = Structs[Pos.y][j + 1];
            
            // Resize
            OriginalLength--;
            Structs[Pos.y].Resize(OriginalLength);
        }
    }
}

void StructsView::SetLayerCutoff(int Cutoff)
{
    this->Cutoff = Cutoff;
}

int StructsView::GetLayerCutoff()
{
    return Cutoff;
}

void StructsView::Render()
{
    // For each layer (bottom to top)
    for(int LayerIndex = 0; LayerIndex < WorldDepth; LayerIndex++)
    {
        // For each item
        int ItemCount = Structs[LayerIndex].GetSize();
        for(int ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++)
        {
            // Convert to float position
            Vector3<int> Pos = Structs[LayerIndex][ItemIndex].Pos;
            
            // Ignore if above the cutoff
            if(Pos.y > GetLayerCutoff() + 1)
                continue;
            
            // Push local translation
            glPushMatrix();
            glTranslatef(Pos.x, Pos.y, Pos.z);
            
                // Render normally
                Structs[LayerIndex][ItemIndex].ModelData->Render();
            
            glPopMatrix();
        }
    }
}

void StructsView::Update(float dT)
{
    // For each layer
    for(int LayerIndex = 0; LayerIndex < WorldDepth; LayerIndex++)
    {
        // For each item
        int ItemCount = Structs[LayerIndex].GetSize();
        for(int ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++)
        {
            // Update the existance time
            Structs[LayerIndex][ItemIndex].dT += dT;
        }
    }
}
