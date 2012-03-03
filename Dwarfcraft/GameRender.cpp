/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "GameRender.h"

GameRender::GameRender(GrfxObject* Parent, Glui2* GluiHandle)
: GrfxObject(Parent)
{
    /*** Load Configuration ***/
    
    g2Config WorldConfig;
    WorldConfig.LoadFile("World.cfg");
    
    // Read the width, depth, and seed of the world
    WorldConfig.GetValue("World", "size", &WorldWidth);
    WorldConfig.GetValue("World", "height", &WorldHeight);
    
    char* WorldSeed = NULL;
    WorldConfig.GetValue("World", "seed", &WorldSeed);
    
    /*** Prepare the scene ***/
    
    // Start with a background
    Background = new BackgroundView(this, -100);
    
    // Create the world buffer (For now, just do an 8x8 column)
    WorldData = new WorldContainer(WorldWidth, WorldHeight, 16);
    
    // Create a clock for performance measuring
    UtilHighresClock Clock;
    
    printf("Generating world data...");
    Clock.Start();
    {
        // Create a world
        WorldGenerator MyWorld;
        MyWorld.Generate(WorldData, WorldSeed);
    }
    Clock.Stop();
    printf(" Total time: %.3fs\n", Clock.GetTime());
    
    // Create the world renderer mechanism
    WorldRender = new WorldView(WorldData);
    
    // Allocate the GUI
    WorldUI = new UserInterface(this, GluiHandle);
    
    // Default to full-style rendering
    RenderStyle = GameRender_RenderStyle_Fill;
    
    /*** Init. Controls ***/
    
    // Turn off all keys
    KeyUp = KeyDown = KeyLeft = KeyRight = false;
    MouseDragging = MouseRotation = ShiftRotating = CtrlRotating = false;
    KeyZoomIn = KeyZoomOut = false;
    
    // Make sure backface culling is on
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Camera has slight angle (radians) and no pitch difference
    CameraRotation = UtilPI / 4.0f; // 90 degrees
    CameraPitchOffset = 0.0f;
    CameraZoom = WorldView_CameraDist;
    
    // Start the camera's target the center of the world, targeting the first non-air block
    CameraTarget = Vector3<float>(WorldData->GetWorldWidth() / 2, 0, WorldData->GetWorldWidth() / 2);
    CameraTarget.y = WorldData->GetSurfaceDepth(CameraTarget.x, CameraTarget.z) + 1;
    
    // Set initial camera pos
    WorldUI->SetPos((int)CameraTarget.x, (int)CameraTarget.y, (int)CameraTarget.z);
    
    // Set the world cutoff at top-most layer and update the slider
    LayerCutoff = WorldData->GetWorldHeight() - 1;
    WorldUI->GetDepthSlider()->SetProgress(0.0f);
    WorldUI->SetDepth(LayerCutoff);
    
    /*** Place Entities ***/
    /*
    // Initialize
    EntitiesList = new Entities(this, WorldData, Designations, Items);
    
    // Select one of four skin colors
    char Skins[4][32] = 
    {
        "Lightest Dwarf.cfg",
        "Lighter Dwarf.cfg",
        "Darker Dwarf.cfg",
        "Darkest Dwarf.cfg"
    };
    
    // Add a dozen entities purely for testing...
    Clock.Start();
    static const int EntityCount = 16;
    for(int i = 0; i < EntityCount; i++)
    {
        // Randomly choose a position
        int x = WorldView_WorldWidth / 2 + i; //rand() % WorldView_WorldWidth;
        int z = WorldView_WorldWidth / 2 + rand() % EntityCount; //rand() % WorldView_WorldWidth;
        int y = WorldData->GetSurfaceDepth(x, z) + 1;
        
        // If this is a half-block, go down one more
        while(!WorldData->GetBlock(x, y, z).IsWhole())
            y--;
        
        // Create a "dumb" AI for test
        DwarfEntity* SampleDwarf = new DwarfEntity(Skins[0]);// Debugging: always same skin for now Skins[i % 4]);
        SampleDwarf->SetPosition(Vector3<int>(x, y, z));
        
        // Explicitly set random armor (chest, pants)
        dItem Chest(dItemType(dItem_ArmorChest_Leather + rand() % 2));
        dItem Boots(dItemType(dItem_ArmorBoots_Leather + rand() % 2));
        SampleDwarf->SetArmor(Chest, Boots);
        
        // Add to entities list
        EntitiesList->AddEntity(SampleDwarf);
    }
    Clock.Stop();
    printf("Time to generate AI: %.3fs\n", Clock.GetTime());
    */
}

GameRender::~GameRender()
{
    // Release world map
    delete WorldData;
}

void GameRender::Render()
{
    // Background is already rendered...
    
    /*** Camera & Render Style ***/
    
    // Compute a view-direction vector (normalized)
    Vector3<float> CameraSource = GetCameraSource();
    gluLookAt(CameraSource.x, CameraSource.y, CameraSource.z, CameraTarget.x, CameraTarget.y, CameraTarget.z, 0.0f, 1.0f, 0.0f);
    
    // Change the render style
    glColor3f(1.0f, 1.0f, 1.0f);
    if(RenderStyle == GameRender_RenderStyle_Point)
        glPolygonMode(GL_FRONT, GL_POINT);
    else if(RenderStyle == GameRender_RenderStyle_Lines)
    {
        glPolygonMode(GL_FRONT, GL_LINE);
        glLineWidth(1.0f);
    }
    else if(RenderStyle >= GameRender_RenderStyle_Fill)
        glPolygonMode(GL_FRONT, GL_FILL);
    
    /*** Render World ***/
    
    // Draw the world about the camera (note that we move the camera further away a little more)
    Vector3<float> RightDirection(cos(CameraRotation - UtilPI / 2.0f), 0, sin(CameraRotation - UtilPI / 2.0f));
    WorldRender->Render(CameraSource, RightDirection, LayerCutoff);
    
    /*** Render Breaking Blocks ***/
    /*
    // For each entity
    Queue<Entity*> Entities = EntitiesList->GetEntities();
    while(!Entities.IsEmpty())
    {
        // Get entity
        Entity* Obj = Entities.Dequeue();
        
        // Is this entity breaking a block?
        Vector3<int> Target; int Step;
        if(Obj->GetBreaking(&Target, &Step) && Target.y <= GetLayerCutoff())
        {
            // Compute the texture
            float srcx, srcy, srcw, srch;
            dGetBlockTexture(dBlockType_Breaking, dBlockFace_Top, &srcx, &srcy, &srcw, &srch);
            
            // Offset based on progress
            srcx += srcw * float(Step);
            
            // Draw a cube
            glPushMatrix();
            
            // The small offsets are to make the breaking cube right above the target cube
            glTranslatef(Target.x - 0.01f, Target.y - 0.01f, Target.z - 0.01f);
            glScalef(1.02f, 1.02f, 1.02f);
            glColor4f(1, 1, 1, 0.5f);
            
            // For each surface
            for(int i = 0; i < 5; i++)
            {
                // Draw surface
                glEnable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glTexCoord2f(srcx, srcy); glVertex3f(GameRender_FaceQuads[i][0].x, GameRender_FaceQuads[i][0].y, GameRender_FaceQuads[i][0].z);
                glTexCoord2f(srcx + srcw, srcy); glVertex3f(GameRender_FaceQuads[i][1].x, GameRender_FaceQuads[i][1].y, GameRender_FaceQuads[i][1].z);
                glTexCoord2f(srcx + srcw, srcy + srch); glVertex3f(GameRender_FaceQuads[i][2].x, GameRender_FaceQuads[i][2].y, GameRender_FaceQuads[i][2].z);
                glTexCoord2f(srcx, srcy + srch); glVertex3f(GameRender_FaceQuads[i][3].x, GameRender_FaceQuads[i][3].y, GameRender_FaceQuads[i][3].z);
                glEnd();
                glDisable(GL_TEXTURE_2D);
            }
            
            glPopMatrix();
        }
    }
    */
    /*** Render Designations ***/
    /*
    // Draw/update each designation
    Designations->SetLayerCutoff(GetLayerCutoff());
    Designations->SetWindowSize(WindowWidth, WindowHeight);
    Designations->Render(RenderDesignationsFlag);
    
    // Update items (misc.)
    Items->SetLayerCutoff(GetLayerCutoff());
    
    // Update structs
    Structs->SetLayerCutoff(GetLayerCutoff());
    */
    /*** Render Selections ***/
    /*
    // Draw selection
    if(AllowSelection && (IsSelecting || HasSelection))
    {
        // Flip around the first and last selections so that first is always the smalest variables
        Vector3<float> P1((FirstSelection.x <= LastSelection.x) ? FirstSelection.x : LastSelection.x,
                          (FirstSelection.y <= LastSelection.y) ? FirstSelection.y : LastSelection.y,
                          (FirstSelection.z <= LastSelection.z) ? FirstSelection.z : LastSelection.z);
        Vector3<float> P2((FirstSelection.x > LastSelection.x) ? FirstSelection.x : LastSelection.x,
                          (FirstSelection.y > LastSelection.y) ? FirstSelection.y : LastSelection.y,
                          (FirstSelection.z > LastSelection.z) ? FirstSelection.z : LastSelection.z);
        
        // Compute the size of the volume
        Vector3<float> Volume(fabs(P1.x - P2.x) + 1, fabs(P1.y - P2.y) + 1, fabs(P1.z - P2.z) + 1);
        
        // Pulse for the color
        float Pulse = 0.1f * sin(PulseTime * 2.0f);
        
        // Draw the cube
        glPushMatrix();
        glTranslatef(P1.x + Volume.x / 2.0f, P1.y + Volume.y / 2.0f, P1.z + Volume.z / 2.0f);
        glScalef(Volume.x + 0.1f, Volume.y + 0.1f, Volume.z + 0.1f);
        
        glColor3f(0.2f + Pulse, 0.2f + Pulse, 0.2f + Pulse);
        glLineWidth(1.0f);
        glutWireCube(1.0f);
        
        glPopMatrix();
    }
    */
}

void GameRender::Update(float dT)
{
    // Update camera based on depth
    const float SpeedRatio = 2000.0f;
    
    /*** User Control Updates ***/
    
    // Compute the direction (note that the ViewDirection.y is an alias to z);
    Vector2<float> FrontDirection(cos(CameraRotation), sin(CameraRotation));
    FrontDirection.Normalize();
    
    // Computer the left direction (right is the same, just multiplied by -1, since that flips it 180-degrees)
    Vector2<float> RightDirection(cos(CameraRotation - UtilPI / 2.0f), sin(CameraRotation - UtilPI / 2.0f));
    RightDirection.Normalize();
    
    // Compute movement speed
    const float Fovy = 60;
    float MoveFrontRatio = ((float(WindowHeight) / float(WindowWidth)) / float(Fovy)) * SpeedRatio;
    float MoveSideRatio = ((float(WindowWidth) / float(WindowHeight)) / float(Fovy)) * SpeedRatio;
    
    // Update camera movement
    if(KeyUp)
    {
        CameraTarget.x -= FrontDirection.x * dT * MoveFrontRatio;
        CameraTarget.z -= FrontDirection.y * dT * MoveFrontRatio;
    }
    if(KeyDown)
    {
        CameraTarget.x += FrontDirection.x * dT * MoveFrontRatio;
        CameraTarget.z += FrontDirection.y * dT * MoveFrontRatio;
    }
    if(KeyLeft)
    {
        CameraTarget.x -= RightDirection.x * dT * MoveSideRatio;
        CameraTarget.z -= RightDirection.y * dT * MoveSideRatio;
    }
    if(KeyRight)
    {
        CameraTarget.x += RightDirection.x * dT * MoveSideRatio;
        CameraTarget.z += RightDirection.y * dT * MoveSideRatio;
    }
    
    // If there were any movement changes, bounds check so that the target is always within world bounds
    if(KeyUp || KeyDown || KeyLeft || KeyRight)
    {
        CameraTarget.x = fmax(fmin(CameraTarget.x, WorldData->GetWorldWidth() - 1), 0);
        CameraTarget.z = fmax(fmin(CameraTarget.z, WorldData->GetWorldWidth() - 1), 0);
    }
    
    // Target camera depth, the depth that the camera target should be moving
    // towards (though the current actual target is still CameraTarget.y)
    int CameraTargetDepth = WorldData->GetSurfaceDepth(CameraTarget.x, LayerCutoff, CameraTarget.z) + 1;
    bool DepthChanged = false;
    if((int)CameraTarget.y != CameraTargetDepth)
    {
        DepthChanged = true;
        CameraTarget.y += ((float)CameraTargetDepth - CameraTarget.y) * dT * 5.0f;
    }
    
    // Change zoom
    if(KeyZoomIn)
    {
        CameraZoom = fmax(5, CameraZoom - 0.3f); // 1/4th of middle
        WorldUI->GetFovySlider()->SetProgress((CameraZoom - 5) / 35);
    }
    if(KeyZoomOut)
    {
        CameraZoom = fmin(40, CameraZoom + 0.3f); // 2x max
        WorldUI->GetFovySlider()->SetProgress((CameraZoom - 5) / 35);
    }
    
    /*** UI Updates ***/
    
    // If there were any movements, update the position text
    if(KeyUp || KeyDown || KeyLeft || KeyRight || DepthChanged)
        WorldUI->SetPos((int)CameraTarget.x, (int)CameraTarget.y, (int)CameraTarget.z);
    
    // Change depth based on slider
    LayerCutoff = (1.0f - WorldUI->GetDepthSlider()->GetProgress()) * (WorldData->GetWorldHeight() - 1);
    WorldUI->SetDepth(LayerCutoff);
    
    // Get the slider's zoom
    CameraZoom = WorldUI->GetFovySlider()->GetProgress() * 35 + 5;
    
    /*** Data Updates ***/
    
    // Commit camera rotations for AI & Items; note that the angle
    // is given as negative since we are pointing to, not away,
    // the entities
    //EntitiesList->SetCameraAngle(-CameraRotation);
    //Items->SetCameraAngle(-CameraRotation);
    
    // Update renderer if needed
    WorldRender->Update(dT);
    
    // Update designations
    Designations->Update(dT);
}

void GameRender::WindowResizeEvent(int NewWidth, int NewHeight)
{
    // Save new size
    WindowWidth = NewWidth;
    WindowHeight = NewHeight;
}

void GameRender::KeyboardEventDown(unsigned char key, int x, int y)
{
    // Move camera state check
    key = tolower(key);
    if(key == 'w')
        KeyUp = true;
    else if(key == 's')
        KeyDown = true;
    else if(key == 'd')
        KeyRight = true;
    else if(key == 'a')
        KeyLeft = true;
    
    // Zoom and depth changes
    else if(key == 'c')
        WorldUI->GetDepthSlider()->SetProgress(WorldUI->GetDepthSlider()->GetProgress() + 1.0f / float(WorldData->GetWorldHeight() - 1));
    else if(key == 'e')
        WorldUI->GetDepthSlider()->SetProgress(WorldUI->GetDepthSlider()->GetProgress() - 1.0f / float(WorldData->GetWorldHeight() - 1));
    else if(key == 'z')
        KeyZoomIn = true;
    else if(key == 'x')
        KeyZoomOut = true;
}

void GameRender::KeyboardEventUp(unsigned char key, int x, int y)
{
    // Move camera state check
    key = tolower(key);
    if(key == 'w')
        KeyUp = false;
    else if(key == 's')
        KeyDown = false;
    else if(key == 'd')
        KeyRight = false;
    else if(key == 'a')
        KeyLeft = false;
    
    // Zoom and depth changes
    else if(key == 'z')
        KeyZoomIn = false;
    else if(key == 'x')
        KeyZoomOut = false;
}

void GameRender::MouseEvent(int button, int state, int x, int y)
{
    // Save modifiers
    int Modifiers = glutGetModifiers();
    
    // Get the user's selection ray
    Vector3<float> ViewOrigin, ViewDirection;
    //GetUserSelectionRay(x, y, &ViewOrigin, &ViewDirection);
    
    // If the right mouse button is pressing down, we are dragging
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        // Save mouse info and start dragging
        MouseStartX = x;
        MouseStartY = y;
        MouseDragging = true;
        
        // If we are actually rotating...
        if((Modifiers & GLUT_ACTIVE_CTRL) == GLUT_ACTIVE_CTRL || (Modifiers & GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_SHIFT)
        {
            // Center mouse position
            MouseStartX = WindowWidth / 2;
            MouseStartY = WindowHeight / 2;
            MouseRotation = true;
            //MouseFirstDrag = true;
            
            // Rotate about the mouse's selection
            Vector3<int> Hit;
            /*if(WorldData->IntersectWorld(ViewOrigin, ViewDirection, LayerCutoff, &Hit))
            {
                // Convert the hit point into a float for easier maths
                Vector3<float> fHit(Hit.x, Hit.y, Hit.z);
                
                // Distance from current camera target to where we hit
                float Distance = Vector3<float>(fHit - GetCameraPos()).GetLength();
                
                // Based on the look vector, compute a new camera target
                Vector3<float> CamDir = (CameraTarget - GetCameraPos());
                CamDir.Normalize();
                CameraTarget = GetCameraPos() + CamDir * Distance;
            }*/
            
            // Only go into perspective if active ctrl
            //if((Modifiers & GLUT_ACTIVE_CTRL) == GLUT_ACTIVE_CTRL)
            //    MainWindow->Use3DPerspective(true);
        }
    }
    else if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
    {
        MouseDragging = false;
        MouseRotation = false;
        //MainWindow->Use3DPerspective(false);
    }
    /*
    // World or entity selection started by user
    else if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        // Start the world selection (if in volume selection mode)
        if(AllowSelection)
        {
            // Attempt to find in the main world data
            if(WorldData->IntersectWorld(ViewOrigin, ViewDirection, LayerCutoff, &FirstSelection))
            {
                LastSelection = FirstSelection;
                IsSelecting = true;
            }
        }
        // Entity selection since we aren't doing volue selection
        else
            SelectedEntity = EntitiesList->IntersectEntities(ViewOrigin, ViewDirection, LayerCutoff);
    }
    
    // Done selecting
    else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        // If we are doing world selection
        if(AllowSelection && IsSelecting)
        {
            // Done selecting
            IsSelecting = false;
            
            // If this last point is valid, save it
            Vector3<int> Collision;
            if(WorldData->IntersectWorld(ViewOrigin, ViewDirection, LayerCutoff, &Collision))
            {
                LastSelection = Collision;
                HasSelection = true;
            }
            else
                HasSelection = false;
        }
    }
    */
}

void GameRender::PassiveMouseEvent(int x, int y)
{
    // Rotation
    if(MouseDragging && MouseRotation)
    {
        // Change ratio and mouse delta
        static const float SpeedRatio = 0.001f;
        int dx = x - MouseStartX;
        int dy = y - MouseStartY;
        /*
        // The mouse pisition jumps when we do the initial drag
        if(MouseFirstDrag)
        {
            dx = 0;
            dy = 0;
            MouseFirstDrag = false;
        }
        */
        // Apply rotation and then re-center mouse
        CameraRotation += float(dx) * SpeedRatio;
        
        // Apply pitch change; but bounded to [-1, 1]
        CameraPitchOffset += float(dy) * SpeedRatio;
        if(CameraPitchOffset > 1.0f)
            CameraPitchOffset = 1.0f;
        else if(CameraPitchOffset < -1.0f)
            CameraPitchOffset = -1.0f;
    }
    /*
    // Translate
    else if(MouseDragging)
    {
        // Change ratio (Note: May need change based on dept of camera)
        static const float SpeedRatio = 2.5f;
        
        // Compute the direction (note that the ViewDirection.y is an alias to z);
        Vector2<float> FrontDirection(cos(CameraRotation), sin(CameraRotation));
        FrontDirection.Normalize();
        
        // Computer the left direction (right is the same, just multiplied by -1, since that flips it 180-degrees)
        Vector2<float> LeftDirection(cos(CameraRotation - UtilPI / 2.0f), sin(CameraRotation - UtilPI / 2.0f));
        LeftDirection.Normalize();
        
        // Get fovy
        const float Fovy = MainWindow->GetFovy();
        
        // Move left/right
        float MoveRatio = float(x - MouseX) * ((float(WindowHeight) / float(WindowWidth)) / float(Fovy)) * SpeedRatio;
        CameraTarget.x += LeftDirection.x * MoveRatio;
        CameraTarget.z += LeftDirection.y * MoveRatio;
        
        // Move up/down
        MoveRatio = float(y - MouseY) * ((float(WindowWidth) / float(WindowHeight)) / float(Fovy)) * SpeedRatio;
        CameraTarget.x += FrontDirection.x * MoveRatio;
        CameraTarget.z += FrontDirection.y * MoveRatio;
    }
    
    // Note that we do the collision detection in the update
    */
    // Save the mouse drags for the next movement
    MouseStartX = x;
    MouseStartY = y;
}

Vector3<float> GameRender::GetCameraSource()
{
    // Create a vector that is rotated about the center
    Vector3<float> CameraSource(cos(CameraRotation), 0.7f + CameraPitchOffset, sin(CameraRotation));
    
    // Grow the vector away
    return CameraTarget + CameraSource * CameraZoom;
}
