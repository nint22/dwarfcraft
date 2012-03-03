/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "DesignationsView.h"

void dGetDesignationTexture(DesignationType Type, float* x, float* y, float* width, float* height)
{
    // Get the total width and height
    int TextureWidth, TextureHeight, TileSize;
    dGetTerrainTextureID(&TextureWidth, &TextureHeight, &TileSize);
    
    // Get the appropriate base-face location
    *x = float(dDesignationTexturePos.x * TileSize) / float(TextureWidth);
    *y = float(dDesignationTexturePos.y * TileSize) / float(TextureHeight);
    *width = float(TileSize) / float(TextureWidth);
    *height = float(TileSize) / float(TextureHeight);
    
    // Offset based on tye (firs type is 0)
    *x += *width * int(Type);
}

DesignationsView::DesignationsView(WorldContainer* MainWorld)
{
    // Save world data
    WorldData = MainWorld;
}

DesignationsView::~DesignationsView()
{
    // Nothing to release...
}

void DesignationsView::AddDesignation(DesignationType Type, Vector3<int> Origin, Vector3<int> Volume)
{
    // Struct to insert
    Designation ToInsert;
    ToInsert.Type = Type;
    ToInsert.Origin = Origin;
    ToInsert.Volume = Volume;
    
    // Add to queue
    DesignationsQueue.Enqueue(ToInsert);
}

void DesignationsView::RemoveDesignation(Vector3<int> Point)
{
    // Keep cycling through until we find the one we want to remove
    int DesignationsCount = DesignationsQueue.GetSize();
    for(int i = 0; i < DesignationsCount; i++)
    {
        // Pop off to peek
        Designation Area = DesignationsQueue.Dequeue();
        
        // If this is the target point, don't add back
        if(Area.Origin != Point)
            DesignationsQueue.Enqueue(Area);
    }
}

Queue< std::pair< Vector3<int>, Vector3<int> > > DesignationsView::FindDesignation(DesignationType Type, Vector3<int> Origin)
{
    // List of empty spaces we may or may not be able to access
    Queue< std::pair< Vector3<int>, Vector3<int> > > ToVisit;
    
    /*** Find Closest Designation ***/
    
    // Current closest valid designation
    Designation TargetDesignation;
    float Distance = INFINITY;
    
    // Keep cycling through until we find the one we want to remove
    int DesignationsCount = DesignationsQueue.GetSize();
    for(int i = 0; i < DesignationsCount; i++)
    {
        // Pop off to peek
        Designation Area = DesignationsQueue.Dequeue();
        
        // If filtered type
        if(Area.Type == Type)
        {
            // Find the center of this area
            Vector3<int> Center(Area.Origin + Area.Volume / 2);
            
            // Distance from origin to center
            float ThisDistance = Vector3<int>(Center - Origin).GetLength();
            if(ThisDistance < Distance)
            {
                Distance = ThisDistance;
                TargetDesignation = Area;
            }
        }
        
        // Add area back
        DesignationsQueue.Enqueue(Area);
    }
    
    // If we found nothing, return an empty list because there are no designations of this type
    if(Distance == INFINITY)
        return ToVisit;
    
    /*** Find Valid Blocks ***/
    
    // List of all directly adjacent blocks to check
    static const int OffsetCount = 6;
    static const Vector3<int> Offsets[OffsetCount] = {
        Vector3<int>(1, 0, 0),
        Vector3<int>(-1, 0, 0),
        Vector3<int>(0, 1, 0),
        Vector3<int>(0, -1, 0),
        Vector3<int>(0, 0, 1),
        Vector3<int>(0, 0, -1),
    };
    
    // For all designation blocks
    for(int y = TargetDesignation.Origin.y; y < TargetDesignation.Origin.y + TargetDesignation.Volume.y; y++)
    for(int z = TargetDesignation.Origin.z; z < TargetDesignation.Origin.z + TargetDesignation.Volume.z; z++)
    for(int x = TargetDesignation.Origin.x; x < TargetDesignation.Origin.x + TargetDesignation.Volume.x; x++)
    {
        // This adjacent block must be open at the <x, y, z> + Offset and solid right below that
        for(int i = 0; i < OffsetCount; i++)
        {
            // Target: block we want to work on, source: where the dwarf will be
            Vector3<int> Target(x, y, z);
            Vector3<int> Source = Target + Offsets[i];
            
            // Make sure in bounds (if out of bounds, ignore)
            if(WorldData->IsWithinWorld(Target.x, Target.y, Target.z) && WorldData->IsWithinWorld(Source.x, Source.y, Source.z))
            {
                // If the target is solid (what we want to remove) and the source is air (where the dwarf will be), then it is a probably valid designation position
                // Note: we say "probably" becuase the source block may not be accessable, but that is left to the path-planner to decide
                if(dIsSolid(WorldData->GetBlock(Target.x, Target.y, Target.z)) && WorldData->GetBlock(Source.x, Source.y, Source.z).GetType() == dBlockType_Air)
                    ToVisit.Enqueue(std::pair< Vector3<int>, Vector3<int> >(Target, Source)); // <Block to change, block we should be in>
            }
        }
    }
    
    /*** Special Cases ***/
    
    // Special case: if we are mining and the visiting list is empty, we look for other mining tasks in another designation
    if(Type == DesignationType_Mine && ToVisit.IsEmpty())
    {
        // Remove this designation
        int DesignationsCount = DesignationsQueue.GetSize();
        for(int i = 0; i < DesignationsCount; i++)
        {
            // Keep pushing back unless we find our target area
            Designation Area = DesignationsQueue.Dequeue();
            if(Area.Type == TargetDesignation.Type && Area.Origin == TargetDesignation.Origin && Area.Volume == TargetDesignation.Volume)
                break;
            DesignationsQueue.Enqueue(Area);
        }
        
        // Attempt to find again (recursive)
        ToVisit = FindDesignation(Type, Origin);
    }
    
    // Return our list of open spaces
    return ToVisit;
}

Queue< Designation > DesignationsView::GetDesignations()
{
    return DesignationsQueue;
}

void DesignationsView::GetDesignationsStrings(List< DesignationType >** DesignationTypes, List< Vector2<int> >** ScreenPositions)
{
    // Copy our own internal copies if we are rendering designations
    *DesignationTypes = &SavedDesignationTypes;
    *ScreenPositions = &SavedScreenPositions;
}

void DesignationsView::Update(float dT)
{
    // Do nothing...
}

void DesignationsView::Render(int LayerCutoff, bool Draw)
{
    // If we are drawing..
    if(Draw)
    {
        // For each designations
        Queue< Designation > DesignationQueue = GetDesignations();
        SavedDesignationTypes.Resize(DesignationQueue.GetSize());
        SavedScreenPositions.Resize(DesignationQueue.GetSize());
        
        for(int i = 0; !DesignationQueue.IsEmpty(); i++)
        {
            // Get active area
            Designation Area = DesignationQueue.Dequeue();
            
            // Push what we will work on
            glPushMatrix();
            
            // Move and scale as needed (shouldn't it scale first then translate?)
            Vector3<float> VolumeCenter(Area.Origin.x + Area.Volume.x / 2.0f, Area.Origin.y + Area.Volume.y / 2.0f, Area.Origin.z + Area.Volume.z / 2.0f);
            glTranslatef(VolumeCenter.x, VolumeCenter.y, VolumeCenter.z);
            glScalef(Area.Volume.x + 0.2f, Area.Volume.y + 0.2f, Area.Volume.z + 0.2f);
            
            // Draw cube
            glColor3f(0.2f, 0.2f, 0.2f);
            glLineWidth(2.0f);
            glutWireCube(1.0f);
            
            // Pop this localized coordinate
            glPopMatrix();
            
            // Get texture info
            float srcx, srcy, srcwidth, srcheight;
            dGetDesignationTexture(Area.Type, &srcx, &srcy, &srcwidth, &srcheight);
            
            glEnable(GL_TEXTURE_2D);
            
            static float phase = 0.0f;
            glColor3f(1, 1, 1.0f + 0.2f * sin(phase += 0.1f));
            
            // For each valid tile we can render on...
            for(int y = Area.Origin.y; y < Area.Origin.y + Area.Volume.y; y++)
            for(int z = Area.Origin.z; z < Area.Origin.z + Area.Volume.z; z++)
            for(int x = Area.Origin.x; x < Area.Origin.x + Area.Volume.x; x++)
            {
                // Only render on this block if it is solid and above is air
                if(y <= LayerCutoff && dIsSolid(WorldData->GetBlock(x, y, z)) && WorldData->GetBlock(x, y + 1, z).GetType() == dBlockType_Air)
                {
                    // Move down 0.5f if half block
                    float VerticalOffset = 0.0f;
                    if(!WorldData->GetBlock(x, y, z).IsWhole())
                        VerticalOffset = -0.5f;
                    
                    // Render a mining tile
                    // Note the slight shift upwards because we want to render it ABOVE a block
                    glBegin(GL_QUADS);
                    glTexCoord2f(srcx + srcwidth, srcy); glVertex3f(x + 0.0f, y + 1.01f + VerticalOffset, z + 0.0f);
                    glTexCoord2f(srcx, srcy); glVertex3f(x + 0.0f, y + 1.01f + VerticalOffset, z + 1.0f);
                    glTexCoord2f(srcx, srcy + srcheight); glVertex3f(x + 1.0f, y + 1.01f + VerticalOffset, z + 1.0f);
                    glTexCoord2f(srcx + srcwidth, srcy + srcheight); glVertex3f(x + 1.0f, y + 1.01f + VerticalOffset, z + 0.0f);
                    glEnd();
                }
            }
            
            glDisable(GL_TEXTURE_2D);
            
            // Save the type and screen position
            SavedDesignationTypes[i] = Area.Type;
            SavedScreenPositions[i] = WorldToScreen(VolumeCenter);
        }
    }
    else
    {
        // Since we aren't rendering anymore, make sure we resize to length 0
        SavedDesignationTypes.Resize(0);
        SavedScreenPositions.Resize(0);
    }
}

void DesignationsView::SetWindowSize(int Width, int Height)
{
    // Ignore width, save window height
    WindowHeight = Height;
}

Vector2<int> DesignationsView::WorldToScreen(Vector3<float> Position)
{
    // Arrays to hold matrix information
    GLdouble model_view[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
    
    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Get 2D screen position based on given position
    GLdouble x, y, z;
    gluProject(Position.x, Position.y, Position.z, model_view, projection, viewport, &x, &y, &z);
    return Vector2<int>(x, WindowHeight - y);
}
