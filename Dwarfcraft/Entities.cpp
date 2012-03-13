/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "Entities.h"

Entities::Entities(WorldContainer* MainWorld, DesignationsView* MainDesignations, ItemsView* MainItems)
{
    Theta = 0.0f;
    this->MainWorld = MainWorld;
    this->MainDesignations = MainDesignations;
    this->MainItems = MainItems;
    
    // Default to not rendering the path
    RenderablePath = true;
}

Entities::~Entities()
{
    // Nothing to release...
}

void Entities::AddEntity(Entity* NewEntity)
{
    // Apply existing known angle and world data
    NewEntity->MainWorld = MainWorld;
    NewEntity->Designations = MainDesignations;
    NewEntity->Items = MainItems;
    EntitiesList.Resize(EntitiesList.GetSize() + 1);
    EntitiesList[EntitiesList.GetSize() - 1] = NewEntity;
}

Queue< Entity* > Entities::GetEntities()
{
    Queue< Entity* > EntitiesQueue;
    int EntityCount = EntitiesList.GetSize();
    for(int i = 0; i < EntityCount; i++)
        EntitiesQueue.Enqueue(EntitiesList[i]);
    return EntitiesQueue;
}

void Entities::SetRenderingPath(bool Set)
{
    RenderablePath = Set;
}

bool Entities::GetRenderingPath()
{
    return RenderablePath;
}

Entity* Entities::IntersectEntities(Vector3<float> RayPos, Vector3<float> RayDir, int CutoffLayer)
{
    // All intersecting entities; we will have to sort to find the closest
    Queue<Entity*> Intersections;
    
    // For each entity
    int EntityCount = EntitiesList.GetSize();
    for(int i = 0; i < EntityCount; i++)
    {
        // Get the entity
        Entity* Obj = EntitiesList[i];
        
        // Make some variables arrays for easy access (index maps to x,y,z)
        float _BoxPos[3] = { Obj->GetPosition().x, Obj->GetPosition().y, Obj->GetPosition().z };
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
        if(IsValid)
            Intersections.Enqueue(Obj);
    }
    
    // Find the closest intersection
    float Dist = INFINITY;
    Entity* Closest = NULL;
    
    while(!Intersections.IsEmpty())
    {
        // Get next entity
        Entity* Obj = Intersections.Dequeue();
        float ObjDist = (Obj->GetPosition() - RayPos).GetLength();
        
        if(ObjDist < Dist)
        {
            Dist = ObjDist;
            Closest = Obj;
        }
    }
    
    // Return the closest (or null if the intersections list was empty)
    return Closest;
}

void Entities::Update(float dT)
{
    // Update all
    int EntitiesCount = EntitiesList.GetSize();
    for(int i = 0; i < EntitiesCount; i++)
        EntitiesList[i]->__Update(dT);
}

void Entities::Render(int LayerCutoff, float CameraAngle)
{
    // Render those below the cutoff
    int EntitiesCount = EntitiesList.GetSize();
    for(int i = 0; i < EntitiesCount; i++)
    {
        if(EntitiesList[i]->GetPosition().y <= LayerCutoff)
            EntitiesList[i]->__Render(CameraAngle);
    }
    
    // Define line properties
    glLineWidth(3.0f);
    glEnable(GL_LINE_SMOOTH);
    
    // Draw the path data of each entity (if that entity is moving)
    for(int i = 0; i < EntitiesCount; i++)
    {
        // Get entity
        Entity* Obj = EntitiesList[i];
        
        // Only render if we are rendering the global paths OR if we are rendering this entities path
        if(RenderablePath || Obj->GetPathRender())
        {
            // Get starting position and path
            Vector3<float> EntityPos = Obj->GetPosition();
            Stack< Vector3<int> > MovingPath = Obj->GetMovingPath();
            
            // Set color
            float r, g, b;
            g2Label::GetTemplateColor(Obj->GetEntityID() % 16, &r, &g, &b);
            glColor3f(r * 0.5f, g * 0.5f, b * 0.5f);
            
            // Get the list of movements for this obj
            glBegin(GL_LINE_STRIP);
            
            // Initial point is the entitie's current position
            glVertex3f(EntityPos.x, EntityPos.y + 0.1f, EntityPos.z);
            
            // For each block we need to visit
            while(!MovingPath.IsEmpty())
            {
                // Only render if visible path
                Vector3<int> Pos = MovingPath.Pop();
                if(Pos.y <= LayerCutoff + 1)
                {
                    // Get the half/whole state of this block
                    bool IsPosWhole = MainWorld->GetBlock(Pos).IsWhole();
                    
                    float VOffset = 0.0f;
                    if(!IsPosWhole)
                        VOffset += 0.5f;
                    
                    // Center a point ontop of the middle of the block and render point
                    glVertex3f(Pos.x + 0.5f, Pos.y + 0.1f + VOffset, Pos.z + 0.5f);
                    
                    // If there is a next block, peek at it and render a point of transition
                    if(!MovingPath.IsEmpty())
                    {
                        // Look at the next position
                        Vector3<int> Next = MovingPath.Peek();
                        
                        // Get the half/whole state of the next block
                        bool IsNextWhole = MainWorld->GetBlock(Next).IsWhole();
                        
                        // If the next position is above us, then the point needs to go high
                        if(Next.y >= Pos.y && IsPosWhole && !IsNextWhole)
                            glVertex3f(Pos.x + 0.5f, Pos.y + 0.1f + VOffset + 0.5f, Pos.z + 0.5f);
                        else if(Next.y > Pos.y && !IsPosWhole && IsNextWhole)
                            glVertex3f(Pos.x + 0.5f, Pos.y + 0.1f + VOffset + 0.5f, Pos.z + 0.5f);
                        else if(Next.y <= Pos.y && !IsPosWhole && IsNextWhole)
                            glVertex3f(Next.x + 0.5f, Pos.y + 0.1f + VOffset, Next.z + 0.5f);
                        else if(Next.y <= Pos.y && IsPosWhole && !IsNextWhole)
                            glVertex3f(Next.x + 0.5f, Pos.y + 0.1f, Next.z + 0.5f);
                    }
                }
                else
                {
                    // Else, we are starting a new line loop
                    glBegin(GL_LINE_STRIP);
                    glEnd();
                }
            }
            glEnd();
        }
    }
    
    // Done rendering paths
    glDisable(GL_LINE_SMOOTH);
}
