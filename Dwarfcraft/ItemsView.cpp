/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "ItemsView.h"

// A buffer between ground and the item, so we have enough space
// to render shadows, etc.
static const float ItemsView_dHover = 0.01f;

ItemsView::ItemsView(WorldContainer* WorldData)
{
    // Save data
    this->WorldData = WorldData;
    
    // Load the terrain texture
    TextureID = dGetItemTextureID();
}

ItemsView::~ItemsView()
{
    // Nothing to release
}

void ItemsView::AddItem(dItem Item, Vector3<int> Pos)
{
    // Grow list size, add to end
    int OriginalLength = Items.GetSize();
    Items.Resize(OriginalLength + 1);
    
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
    Data.Pos = Vector3<float>(Pos.x + 0.5f, Pos.y + 2.0f * ItemsView_dHover, Pos.z + 0.5f); // Cast to float, and start a little higher
    Data.Vel = PopVector; // Initial velocity
    Data.dT = float(rand()) / float(RAND_MAX); // Randomize the initial time
    
    Items[OriginalLength] = Data;
}

void ItemsView::RemoveItem(Vector3<int> Pos)
{
    // TODO... How should an item be idenitified? Pointer? Unique ID?
}

void ItemsView::Render(int LayerCutoff, float CameraAngle)
{
    // Save camera angle
    this->CameraAngle = CameraAngle;
    
    // For each item
    int ItemCount = Items.GetSize();
    for(int ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++)
    {
        // Get the item we are working on
        ItemsView_Item* ItemView = &Items[ItemIndex];
        
        /*** Item Sprite ***/
        
        // Only render if at level or below
        if((int)ItemView->Pos.y <= LayerCutoff)
        {
            // Convert to float position
            dItem ItemType = ItemView->Item;
            Vector3<float> Pos = ItemView->Pos;
            Vector3<float> fPos(Pos.x, Pos.y, Pos.z);
            
            // Get item information
            float x, y, width, height;
            dGetItemTexture(ItemType.Type, &x, &y, &width, &height);
            
            // Render the tile at this given location in 2D
            float dY = 0.02f + 0.02f * sin(ItemView->dT);
            RenderBillboard(fPos + Vector3<float>(0, dY, 0), x, y, width, height);
        }
        
        /*** Shadow ***/
        
        // Get the surface the item is on
        float GroundSurface = WorldData->GetSurfaceDepth(ItemView->Pos.x, ItemView->Pos.y, ItemView->Pos.z);
        
        // Is the block a half-block? (we do -1 to look down to the colliding block)
        bool IsWhole = WorldData->GetBlock(ItemView->Pos.x, GroundSurface, ItemView->Pos.z).IsWhole();
        if(IsWhole)
            GroundSurface += 1.0f;
        else
            GroundSurface += 0.5f;
        
        // Render a shadow under it (always at the layer or below)
        if((int)GroundSurface <= LayerCutoff)
            RenderShadow(Vector3<float>(ItemView->Pos.x, GroundSurface, ItemView->Pos.z), 0.2f);
    }
}

void ItemsView::Update(float dT)
{
    // Define an animation speed, so we can make things look faster
    static const float AnimationSpeed = 2.0f;
    
    // For each item
    int ItemCount = Items.GetSize();
    for(int ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++)
    {
        // Get the item we are working on
        ItemsView_Item* ItemView = &Items[ItemIndex];
        
        // Update the existance time
        ItemView->dT += dT * 5.0f;
        
        // Get the surface the item is on
        float GroundSurface = WorldData->GetSurfaceDepth(ItemView->Pos.x, ItemView->Pos.y, ItemView->Pos.z);
        
        // Is the block a half-block? (we do -1 to look down to the colliding block)
        bool IsWhole = WorldData->GetBlock(ItemView->Pos.x, GroundSurface, ItemView->Pos.z).IsWhole();
        if(IsWhole)
            GroundSurface += 1.0f;
        else
            GroundSurface += 0.5f;
        
        // Apply velocity to the position (only if above the surface)
        if(ItemView->Pos.y > GroundSurface + ItemsView_dHover)
        {
            // Increase velocity over time due to acceleration
            ItemView->Vel.y += -2.0f * dT * AnimationSpeed; // Gravity is a fudge-factor
            ItemView->Pos += ItemView->Vel * dT * AnimationSpeed;
        }
        // Else, just reset velocity
        else
            ItemView->Vel = Vector3<float>(); // Set to zero
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
    
    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    
    // Render a 0.5 0.5 bilboard at the target position
    glPushMatrix();
    {
        // Initialize color, rotation, and location
        glColor3f(1, 1, 1);
        
        // Center position
        glTranslatef(pos.x, pos.y, pos.z);
        glRotatef(UtilRadToDeg * (CameraAngle  - UtilPI / 2.0f), 0, 1, 0);
        
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
        glTranslatef(pos.x, pos.y + 0.01f, pos.z);
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
