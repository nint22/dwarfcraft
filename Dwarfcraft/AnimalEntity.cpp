/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "AnimalEntity.h"

AnimalEntity::AnimalEntity(const char* ConfigName)
: Entity(ConfigName)
{
    // Get the max distance and stall times
    // for this specific animal
    GetConfigFile()->GetValue("Animal", "Stall", &MinStall, &MaxStall);
    GetConfigFile()->GetValue("Animal", "Distance", &MinDistance, &MaxDistance);
}

AnimalEntity::~AnimalEntity()
{
    
}

void AnimalEntity::Update(float dT)
{
    // Saves previous operator for this enity (defaults to Idle)
    static EntityOp PreviousOp = EntityOp_Idle;
    
    // Instruction to execute
    EntityInstruction Instruction;
    
    // If no instructions; generate some more
    if(!HasInstructions())
    {
        // Generate a new instruction (or a dozen)
        // Always make sure it's not a repeat of the last instruction
        do
        {
            int OpOffset = rand() % 3;
            Instruction.Operator = EntityOp(EntityOp_Idle + OpOffset);
        } while(Instruction.Operator == PreviousOp);
        
        // Based on the new op, pass args
        if(Instruction.Operator == EntityOp_Idle)
        {
            // Random number of seconds
            Instruction.Data.Idle = MinStall + (float(rand()) / float(RAND_MAX)) * fabs(MaxStall - MinStall);
        }
        else if(Instruction.Operator == EntityOp_MoveTo)
        {
            // Distance we should travel
            int Distance = MinDistance + (float(rand()) / float(RAND_MAX)) * fabs(float(MaxDistance - MinDistance));
            
            // Keep attempting a random position until we find a valid solid block..
            while(true)
            {
                Instruction.Data.Pos.x = GetPosition().x + Distance * ((rand() % 2 == 0) ? 1 : -1);
                Instruction.Data.Pos.z = GetPosition().z + Distance * ((rand() % 2 == 0) ? 1 : -1);
                
                int TopBlock = GetWorld()->GetSurfaceDepth(Instruction.Data.Pos.x, Instruction.Data.Pos.z);
                Instruction.Data.Pos.y = TopBlock + 1;
                
                // If solid block, give up
                if(dIsSolid(GetWorld()->GetBlock(Instruction.Data.Pos.x, TopBlock, Instruction.Data.Pos.z)))
                    break;
            }
        }
        else if(Instruction.Operator == EntityOp_Face)
        {
            // Pick a new random angle to face
            Instruction.Data.Face = (float(rand()) / float(RAND_MAX)) * 2.0f * UtilPI;
        }
        
        // Push into the instruction queue
        PreviousOp = Instruction.Operator;
        AddInstruction(Instruction);
    }
}
