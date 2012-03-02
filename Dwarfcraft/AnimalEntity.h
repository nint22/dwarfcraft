/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: AnimalEntity.h/cpp
 Desc: A very simple dummy-AI; currently used as testing / dev.
 Animals essentially do a cycle of face, idle, move, idle, loop.
 Some animals have different idle times and distance traveled,
 as specified by the "Animal" group in the configuration and
 its "Stall" (fraction of seconds) and "Distance" properties.
 The minimum value of each of these is 1.
 
***************************************************************/

// Inclusion guard
#ifndef __ANIMALENTITY_H__
#define __ANIMALENTITY_H__

#include "Entity.h"
#include "Vector3.h"
#include "WorldGenerator.h"

class AnimalEntity : public Entity
{
public:

    // Constructor and destructor
    AnimalEntity(const char* ConfigName);
    ~AnimalEntity();

protected:
    
    // Update object
    void Update(float dT);

private:

    // Min/Max stall time
    float MinStall, MaxStall;

    // Min/Max distance to travel
    int MinDistance, MaxDistance;

    // True if we are currently executing an operator
    bool IsComplete;

};

// End of inclusion guard
#endif
