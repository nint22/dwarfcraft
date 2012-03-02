/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "ItemsView.h"

ItemsView::ItemsView(GrfxObject* Parent, int WorldDepth)
    : GrfxObject(Parent, 1) // Render AFTER the main content
{
    // Allocate an array of item levels
    ItemLevels = new List< ItemsView_Item >[WorldDepth];
    this->WorldDepth = WorldDepth;
    
    // Set the camera angle to 0
    SetCameraAngle(0.0f);
    
    // Load the terrain texture
    TextureID = dGetItemTextureID();
}

ItemsView::~ItemsView()
{
    // Release the array list
    delete[] ItemLevels;
}

void ItemsView::AddItem(dItem Item, Vector3<int> Pos)
{
    // Grow list size, add to end
    int OriginalLength = ItemLevels[Pos.y].GetSize();
    ItemLevels[Pos.y].Resize(OriginalLength + 1);
    
    // Randomize upward facing "pop" animation vector (random x, z, while y is always up)
    Vector3<float> PopVector(float(rand()) / float(RAND_MAX), 1.0f, float(rand()) / float(RAND_MAX));
    
    // Random dir on the x-z plane (note the half-size, so we stay within the cube)
    PopVector.x *= (rand() % 2) ? -0.5f : 0.5f;
    PopVector.z *= (rand() % 2) ? -0.5f : 0.5f;
    
    // Normalize
    PopVector.Normalize();
    
    // Place to end
    ItemsView_Item Data;
    Data.Item = Item;
    Data.Settled = false; // Initially animating
    Data.Pos = Vector3<float>(Pos.x, Pos.y, Pos.z); // Cast to float
    Data.Vel = PopVector; // Initial velocity
    Data.dT = float(rand()) / float(RAND_MAX); // Randomize the initial time
    
    ItemLevels[Pos.y][OriginalLength] = Data;
}

void ItemsView::RemoveItem(Vector3<int> Pos)
{
    // Seek the list for items at the position
    int OriginalLength = ItemLevels[Pos.y].GetSize();
    for(int i = 0; i < OriginalLength; i++)
    {
        // If found, shift over from right, then resize
        Vector3<float> fPos = ItemLevels[Pos.y][i].Pos;
        if(Vector3<int>(fPos.x, fPos.y, fPos.z) == Pos)
        {
            // Shift over from right
            for(int j = i; j < OriginalLength - 1; j++)
                ItemLevels[Pos.y][j] = ItemLevels[Pos.y][j + 1];
            
            // Resize
            OriginalLength--;
            ItemLevels[Pos.y].Resize(OriginalLength);
        }
    }
}

void ItemsView::SetCameraAngle(float Theta)
{
    CameraTheta = Theta;
}

float ItemsView::GetCameraAngle()
{
    return CameraTheta;
}

void ItemsView::SetLayerCutoff(int Cutoff)
{
    this->Cutoff = Cutoff;
}

int ItemsView::GetLayerCutoff()
{
    return Cutoff;
}

void ItemsView::Render()
{
    // For each layer (bottom to top)
    for(int LayerIndex = 0; LayerIndex < WorldDepth; LayerIndex++)
    {
        // For each item
        int ItemCount = ItemLevels[LayerIndex].GetSize();
        for(int ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++)
        {
            // Convert to float position
            dItem ItemType = ItemLevels[LayerIndex][ItemIndex].Item;
            Vector3<float> Pos = ItemLevels[LayerIndex][ItemIndex].Pos;
            Vector3<float> fPos(Pos.x, Pos.y, Pos.z);
            Vector3<float> fShadowPos(Pos.x, LayerIndex, Pos.z);
            
            // Ignore if above the cutoff
            if(Pos.y > GetLayerCutoff() + 1)
                continue;
            
            // Get item information
            float x, y, width, height;
            dGetItemTexture(ItemType.Type, &x, &y, &width, &height);
            
            // Render the tile at this given location in 2D
            float dY = 0.02f + 0.02f * sin(ItemLevels[LayerIndex][ItemIndex].dT);
            RenderBillboard(fPos + Vector3<float>(0, dY, 0), x, y, width, height);
            
            // Render a shadow under it (always at the layer)
            RenderShadow(fShadowPos, 0.2f);
        }
    }
}

void ItemsView::Update(float dT)
{
    // Define an animation speed, so we can make things look faster
    static const float AnimationSpeed = 2.0f;
    
    // For each layer
    for(int LayerIndex = 0; LayerIndex < WorldDepth; LayerIndex++)
    {
        // For each item
        int ItemCount = ItemLevels[LayerIndex].GetSize();
        for(int ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++)
        {
            // Update the existance time
            ItemLevels[LayerIndex][ItemIndex].dT += dT * 5.0f;
            
            // Apply velocity to the position (only if above the surface)
            if(!ItemLevels[LayerIndex][ItemIndex].Settled)
            {
                // Slow velocity over time..
                ItemLevels[LayerIndex][ItemIndex].Pos += ItemLevels[LayerIndex][ItemIndex].Vel * dT * AnimationSpeed;
                ItemLevels[LayerIndex][ItemIndex].Vel.y += -2.5f * dT * AnimationSpeed; // Gravity is a fudge-factor
                
                // If the position is ever below the associated layer, drop down
                if(ItemLevels[LayerIndex][ItemIndex].Pos.y <= LayerIndex)
                {
                    ItemLevels[LayerIndex][ItemIndex].Settled = true;
                    ItemLevels[LayerIndex][ItemIndex].Pos.y = LayerIndex;
                }
            }
        }
    }
}

void ItemsView::RenderBillboard(Vector3<float> pos, float srcx, float srcy, float srcwidth, float srcheight, float doffset)
{
    // Define texture structs
    float TextureCoords[4][2] =
    {
        {srcx + srcwidth, srcy + srcheight},
        {srcx, srcy + srcheight},
        {srcx, srcy},
        {srcx + srcwidth, srcy},
    };
    
    // Define geometry size
    static const float outwidth = 0.4f;
    static const float outheight = 0.4f;
    
    // Compute the view-ray offsets (so we can order textures)
    Vector2<float> Offset(cos(-GetCameraAngle()), sin(-GetCameraAngle()));
    if(doffset >= 0.001f || doffset <= -0.001f)
        Offset *= doffset;
    else
        Offset = Vector2<float>(0.0f, 0.0f);
    
    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    
    // Render a 0.5 0.5 bilboard at the target position
    glPushMatrix();
    {
        // Initialize color, rotation, and location
        glColor3f(1, 1, 1);
        
        // Center position
        glTranslatef(pos.x + 0.5f + Offset.x, pos.y, pos.z + 0.5f + Offset.y);
        glRotatef(UtilRadToDeg * (GetCameraAngle()  - UtilPI / 2.0f), 0, 1, 0);
        
        // Front facing sprite
        glBegin(GL_QUADS);
        {
            glTexCoord2f(TextureCoords[0][0], TextureCoords[0][1]);
            glVertex3f(-outwidth / 2.0f, 0, 0);
            
            glTexCoord2f(TextureCoords[1][0], TextureCoords[1][1]);
            glVertex3f(outwidth / 2.0f, 0, 0);
            
            glTexCoord2f(TextureCoords[2][0], TextureCoords[2][1]);
            glVertex3f(outwidth / 2.0f, outheight, 0);
            
            glTexCoord2f(TextureCoords[3][0], TextureCoords[3][1]);
            glVertex3f(-outwidth / 2.0f, outheight, 0);
        }
        glEnd();
    }
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void ItemsView::RenderShadow(Vector3<float> pos, float radius)
{
    // Total segments
    static const int Segments = 12;
    
    // Start a circle at the face
    glPushMatrix();
        glTranslatef(pos.x + 0.5f, pos.y + 0.01f, pos.z + 0.5f);
        glColor4f(0, 0, 0, 0.5f);
        
        glBegin(GL_TRIANGLE_FAN);
        
        glVertex3f(0, 0, 0); // Center point
        for(int i = 0; i <= Segments; i++)
        {
            float n = float(i) * (UtilPI / (Segments * 0.5f));
            glVertex3f(-cos(n) * radius, 0, sin(n) * radius);
        }
        
        glEnd();
        
    // Finished
    glPopMatrix();
}
