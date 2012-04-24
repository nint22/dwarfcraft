/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "GameRender.h"

// Min / max zoom distances
static float const GameRender_MinZoom = 40.0f;
static float const GameRender_MaxZoom = 180.0f;
static float const GameRender_ZoomSpeed = 1.5f;

GameRender::GameRender(GrfxWindow* Parent, Glui2* GluiHandle)
: GrfxObject(Parent)
{
    // Save parent handle and default to isometric perspective transform
    MainWindow = Parent;
    IsIsometric = true;
    MainWindow->Use3DPerspective(false);
    
    /*** Set OpenGL Fog ***/
    
    // Set fog to simple default values
    glFogi(GL_FOG_MODE, GL_EXP);
    //glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 1.05f);
    glFogf(GL_FOG_START, 10000.0f);
    glFogf(GL_FOG_END, 20000.0f);
    //glEnable(GL_FOG);
    
    /*** Load Configuration ***/
    
    g2Config WorldConfig;
    WorldConfig.LoadFile("World.cfg");
    
    // Read the width, depth, and seed of the world
    WorldConfig.GetValue("World", "size", &WorldWidth);
    WorldConfig.GetValue("World", "height", &WorldHeight);
    
    char* WorldSeed = NULL;
    WorldConfig.GetValue("World", "seed", &WorldSeed);
    
    // User settings
    int Setting;
    
    GetUserSetting("General", "MouseSensitivity", &Setting, 1000);
    MouseSensitivy = 1.0f / float(Setting);
    
    GetUserSetting("General", "ScrollSpeed", &Setting, 1000);
    MoveSensitivity = float(Setting);
    
    /*** Generate the world ***/
    
    // Start with a background
    Background = new BackgroundView(this, -1);
    
    // Create the world buffer (For now, just do an 8x8 column)
    int ChunkSize;
    GetUserSetting("General", "ChunkSize", &ChunkSize, 16);
    WorldData = new WorldContainer(WorldWidth, WorldHeight, ChunkSize);
    
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
    
    /*** Prepare the renderables ***/
    
    // Create all of the special views
    Items = new ItemsView(WorldData);
    Designations = new VolumeView(WorldData, GluiHandle->GetMainTheme());
    Structs = new StructsView(WorldData);
    EntitiesList = new Entities(WorldData, Designations, Items);
    
    // Create the world renderer mechanism
    WorldRender = new WorldView(WorldData, Designations, Items, Structs, EntitiesList);
    
    // Allocate the GUI
    WorldUI = new UserInterface(this, GluiHandle);
    
    // Pass our designations list explicitly
    WorldUI->GetDashboardController()->SetDesignationsList(Designations);
    
    // Default to full-style rendering
    RenderStyle = GameRender_RenderStyle_Fill;
    
    /*** Init. Controls ***/
    
    // Turn off all keys
    KeyUp = KeyDown = KeyLeft = KeyRight = false;
    MouseDragging = MouseRotation = false;
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
    
    // Default to not selecting anything
    IsSelecting = false;
    
    /*** TESTING: Place Entities ***/
    
    // Select one of four skin colors
    char Skins[4][32] = 
    {
        "SampleDwarf.cfg",
        "Lighter Dwarf.cfg",
        "Darker Dwarf.cfg",
        "Darkest Dwarf.cfg"
    };
    
    // Add a dozen entities purely for testing...
    Clock.Start();
    int EntityCount = 3; // Any variable count while testing the code
    for(int i = 0; i < EntityCount; i++)
    {
        // Randomly choose a position
        int x = WorldData->GetWorldWidth() / 2 + i; //rand() % WorldView_WorldWidth;
        int z = WorldData->GetWorldWidth() / 2 + rand() % EntityCount; //rand() % WorldView_WorldWidth;
        int y = WorldData->GetSurfaceDepth(x, z) + 1;
        	
        // Create a "dumb" AI for test
        DwarfEntity* SampleDwarf = new DwarfEntity(Skins[0]);// Debugging: always same skin for now Skins[i % 4]);
        SampleDwarf->SetPosition(Vector3<float>(x + 0.5f, y - 0.5f, z + 0.5f)); // Center on the tile (dwarf will auto-fall if needed)
        
        // Explicitly set random armor (chest, pants)
        dItem Chest(dItemType(dItem_ArmorChest_Leather + rand() % 2));
        dItem Boots(dItemType(dItem_ArmorBoots_Leather + rand() % 2));
        //SampleDwarf->SetArmor(Chest, Boots);
        
        // Give dwarf some picks
        SampleDwarf->SetItems(dItem_Pickaxe, dItem_None);
        
        // Add to entities list
        EntitiesList->AddEntity(SampleDwarf);
    }
    Clock.Stop();
    printf("Time to generate AI: %.3fs\n", Clock.GetTime());
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
    // If we are in projection mode (not isometric), then we change distance
    // for zooming, otherwise we change Fovy
    Vector3<float> CameraSource, CameraBacked;
    if(!IsIsometric)
    {
        MainWindow->SetFovy(60);
        float Normalized = 1.0f - (CameraZoom - GameRender_MinZoom) / (GameRender_MaxZoom - GameRender_MinZoom);
        CameraSource = GetCameraSource(2 + Normalized * 10.0f);
        CameraBacked = GetCameraSource(2 + Normalized * 10.0f + 8);
    }
    else
    {
        MainWindow->SetFovy(CameraZoom);
        CameraSource = GetCameraSource(WorldView_CameraDist * 5);
        CameraBacked = GetCameraSource(WorldView_CameraDist + 16);
    }
    
    // Change projection matrix to look from the source to the target; up vector is y+
    gluLookAt(CameraSource.x, CameraSource.y, CameraSource.z, CameraTarget.x, CameraTarget.y, CameraTarget.z, 0.0f, 1.0f, 0.0f);
    
    // Camera angle
    float CameraAngle = -CameraRotation + UtilPI;
    Background->SetCameraAngle(CameraAngle, CameraPitchOffset);
    
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
    WorldRender->Render(CameraBacked, RightDirection, LayerCutoff, CameraAngle);
    
    /*** Render Breaking Blocks ***/
    
    // For each entity
    Queue<Entity*> Entities = EntitiesList->GetEntities();
    while(!Entities.IsEmpty())
    {
        // Get entity
        Entity* Obj = Entities.Dequeue();
        
        // Is this entity breaking a block?
        Vector3<int> Target; int Step;
        if(Obj->GetBreaking(&Target, &Step) && Target.y <= LayerCutoff)
        {
            // Is this breaking block a solid or half?
            bool IsBlockSolid = WorldData->GetBlock(Target).IsWhole();
            
            // Compute the texture
            float srcx, srcy, srcw, srch;
            dGetBlockTexture(dBlockType_Breaking, dBlockFace_Top, &srcx, &srcy, &srcw, &srch);
            
            GLuint BlockTexture;
            BlockTexture = dGetTerrainTextureID();
            
            // Offset based on progress
            srcx += srcw * float(Step);
            
            // Draw a cube
            glPushMatrix();
            
            // The small offsets are to make the breaking cube right above the target cube
            glTranslatef(Target.x - 0.01f, Target.y - 0.01f, Target.z - 0.01f);
            glScalef(1.02f, 1.02f, 1.02f);
            glColor3f(1, 1, 1);
            
            // Set texture
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, BlockTexture);
            
            // For each surface
            for(int i = 0; i < 5; i++)
            {
                // Will be cutting the side by half if it's a half block?
                float SideFactor = 1.0f, TextFactor = 1.0f;
                if(!IsBlockSolid)
                {
                    SideFactor = 0.51f; // Overcome the shadow
                    if(i != 0)
                        TextFactor = 0.5f;
                }
                
                // Draw surface
                glBegin(GL_QUADS);
                    glTexCoord2f(srcx, srcy);
                    glVertex3f(WorldView_FaceQuads[i][0].x, WorldView_FaceQuads[i][0].y * SideFactor, WorldView_FaceQuads[i][0].z);
                    glTexCoord2f(srcx + srcw, srcy);
                    glVertex3f(WorldView_FaceQuads[i][1].x, WorldView_FaceQuads[i][1].y * SideFactor, WorldView_FaceQuads[i][1].z);
                    glTexCoord2f(srcx + srcw, srcy + srch * TextFactor);
                    glVertex3f(WorldView_FaceQuads[i][2].x, WorldView_FaceQuads[i][2].y * SideFactor, WorldView_FaceQuads[i][2].z);
                    glTexCoord2f(srcx, srcy + srch * TextFactor);
                    glVertex3f(WorldView_FaceQuads[i][3].x, WorldView_FaceQuads[i][3].y * SideFactor, WorldView_FaceQuads[i][3].z);
                glEnd();
            }
            
            glDisable(GL_TEXTURE_2D);
            
            glPopMatrix();
        }
    }
    
    /*** Render Selections ***/
    
    // Draw selection
    if(WorldUI->GetDashboardController()->IsSelecting())
    {
        // Flip around the first and last selections so that first is always the smalest variables
        Vector3<float> P1((SelectStart.x <= SelectEnd.x) ? SelectStart.x : SelectEnd.x,
                          (SelectStart.y <= SelectEnd.y) ? SelectStart.y : SelectEnd.y,
                          (SelectStart.z <= SelectEnd.z) ? SelectStart.z : SelectEnd.z);
        Vector3<float> P2((SelectStart.x > SelectEnd.x) ? SelectStart.x : SelectEnd.x,
                          (SelectStart.y > SelectEnd.y) ? SelectStart.y : SelectEnd.y,
                          (SelectStart.z > SelectEnd.z) ? SelectStart.z : SelectEnd.z);
        
        // Compute the size of the volume
        Vector3<float> Volume(fabs(P1.x - P2.x) + 1, fabs(P1.y - P2.y) + 1, fabs(P1.z - P2.z) + 1);
        
        // Pulse for the color
        float Pulse = 1.0f;//0.1f * sin(PulseTime * 2.0f);
        
        // Draw the cube
        glPushMatrix();
        glTranslatef(P1.x + Volume.x / 2.0f, P1.y + Volume.y / 2.0f, P1.z + Volume.z / 2.0f);
        glScalef(Volume.x + 0.1f, Volume.y + 0.1f, Volume.z + 0.1f);
        
        glColor3f(0.2f + Pulse, 0.2f + Pulse, 0.2f + Pulse);
        glLineWidth(1.0f);
        glutWireCube(1.0f);
        
        glPopMatrix();
    }
}

void GameRender::Update(float dT)
{
    /*** User Control Updates ***/
    
    // Compute the direction (note that the ViewDirection.y is an alias to z);
    Vector2<float> FrontDirection(cos(CameraRotation), sin(CameraRotation));
    FrontDirection.Normalize();
    
    // Computer the left direction (right is the same, just multiplied by -1, since that flips it 180-degrees)
    Vector2<float> RightDirection(cos(CameraRotation - UtilPI / 2.0f), sin(CameraRotation - UtilPI / 2.0f));
    RightDirection.Normalize();
    
    // Compute movement speed
    const float Fovy = 60;
    float MoveFrontRatio = ((float(WindowHeight) / float(WindowWidth)) / float(Fovy)) * MoveSensitivity;
    float MoveSideRatio = ((float(WindowWidth) / float(WindowHeight)) / float(Fovy)) * MoveSensitivity;
    
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
    if(KeyUp || KeyDown || KeyLeft || KeyRight || MouseDragging)
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
        CameraZoom = fmax(GameRender_MinZoom, CameraZoom - GameRender_ZoomSpeed);
        WorldUI->GetFovySlider()->SetProgress((CameraZoom - GameRender_MinZoom) / (GameRender_MaxZoom - GameRender_MinZoom));
    }
    if(KeyZoomOut)
    {
        CameraZoom = fmin(GameRender_MaxZoom, CameraZoom + GameRender_ZoomSpeed);
        WorldUI->GetFovySlider()->SetProgress((CameraZoom - GameRender_MinZoom) / (GameRender_MaxZoom - GameRender_MinZoom));
    }
    
    /*** UI Updates ***/
    
    // If there were any movements, update the position text
    if(KeyUp || KeyDown || KeyLeft || KeyRight || MouseDragging || DepthChanged)
        WorldUI->SetPos((int)CameraTarget.x, (int)CameraTarget.y, (int)CameraTarget.z);
    
    // Change depth based on slider
    LayerCutoff = (1.0f - WorldUI->GetDepthSlider()->GetProgress()) * (WorldData->GetWorldHeight() - 1);
    WorldUI->SetDepth(LayerCutoff);
    
    // Get the slider's zoom
    CameraZoom = WorldUI->GetFovySlider()->GetProgress() * (GameRender_MaxZoom - GameRender_MinZoom) + GameRender_MinZoom;
    
    /*** Data Updates ***/
    
    // Update renderer if needed
    WorldRender->Update(dT);
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
    else if(key == 'x')
        KeyZoomIn = true;
    else if(key == 'z')
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
    else if(key == 'x')
        KeyZoomIn = false;
    else if(key == 'z')
        KeyZoomOut = false;
}

void GameRender::MouseEvent(int button, int state, int x, int y)
{
    // Ignore all mouse presses that touch the UI
    int gx, gy;
    WorldUI->GetDashboardController()->GetPos(&gx, &gy);
    if(state == GLUT_DOWN && WorldUI->GetDashboardController()->InController(x - gx, y - gy))
        return;
    
    // Save modifiers
    int Modifiers = glutGetModifiers();
    
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
            MouseFirstDrag = true;
            
            // Only go into perspective if active ctrl
            if((Modifiers & GLUT_ACTIVE_CTRL) == GLUT_ACTIVE_CTRL)
            {
                IsIsometric = false;
                MainWindow->Use3DPerspective(true);
            }
        }
    }
    else if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
    {
        MouseDragging = false;
        MouseRotation = false;
        IsIsometric = true;
        MainWindow->Use3DPerspective(false);
    }
    
    // Selection
    if(WorldUI->GetDashboardController()->IsSelecting() && button == GLUT_LEFT_BUTTON)
    {
        // Get the user's selection ray
        Vector3<float> ViewOrigin, ViewDirection;
        GetUserSelectionRay(Vector2<int>(x, y), &ViewOrigin, &ViewDirection);
        
        // Starting selection
        if(state == GLUT_DOWN)
        {
            IsSelecting = true;
            WorldData->IntersectWorld(ViewOrigin, ViewDirection, LayerCutoff, &SelectStart);
            SelectEnd = SelectStart;
        }
        
        // Done with selection
        else if(state == GLUT_UP && IsSelecting)
        {
            IsSelecting = false;
            WorldUI->GetDashboardController()->SetSelectionVolume(SelectStart, SelectEnd);
        }
    }
}

void GameRender::PassiveMouseEvent(int x, int y)
{
    // Rotation
    if(MouseDragging && MouseRotation)
    {
        // Change ratio and mouse delta
        int dx = x - MouseStartX;
        int dy = y - MouseStartY;
        
        // The mouse pisition jumps when we do the initial drag
        if(MouseFirstDrag)
        {
            dx = 0;
            dy = 0;
            MouseFirstDrag = false;
        }
        
        // Apply rotation and then re-center mouse
        CameraRotation += float(dx) * MouseSensitivy;
        
        // Apply pitch change; but bounded to [-1, 1]
        CameraPitchOffset += float(dy) * MouseSensitivy;
        
        // Bounds check the pitch
        if(CameraPitchOffset > 0.15f)
            CameraPitchOffset = 0.15f;
        else if(CameraPitchOffset < -0.25f)
            CameraPitchOffset = -0.25f;
    }
    
    // Translate
    else if(MouseDragging)
    {
        // Compute the direction (note that the ViewDirection.y is an alias to z);
        Vector2<float> FrontDirection(cos(CameraRotation), sin(CameraRotation));
        FrontDirection.Normalize();
        
        // Computer the left direction (right is the same, just multiplied by -1, since that flips it 180-degrees)
        Vector2<float> RightDirection(cos(CameraRotation - UtilPI / 2.0f), sin(CameraRotation - UtilPI / 2.0f));
        RightDirection.Normalize();
        
        // Get fovy
        const float Fovy = MainWindow->GetFovy();
        
        // Move left/right
        float MoveRatio = float(x - MouseStartX) * ((float(WindowHeight) / float(WindowWidth)) / float(Fovy)) * MoveSensitivity * 0.002f;
        CameraTarget.x -= RightDirection.x * MoveRatio;
        CameraTarget.z -= RightDirection.y * MoveRatio;
        
        // Move up/down
        MoveRatio = float(y - MouseStartY) * ((float(WindowWidth) / float(WindowHeight)) / float(Fovy)) * MoveSensitivity * 0.002f;
        CameraTarget.x -= FrontDirection.x * MoveRatio;
        CameraTarget.z -= FrontDirection.y * MoveRatio;
    }
    
    // If we are actively selecting, update some information
    if(WorldUI->GetDashboardController()->IsSelecting() && IsSelecting)
    {
        // Get the user's selection ray
        Vector3<float> ViewOrigin, ViewDirection;
        GetUserSelectionRay(Vector2<int>(x, y), &ViewOrigin, &ViewDirection);
        
        WorldData->IntersectWorld(ViewOrigin, ViewDirection, LayerCutoff, &SelectEnd);
        WorldUI->GetDashboardController()->SetSelectionVolume(SelectStart, SelectEnd);
    }
    
    // Save the mouse drags for the next movement
    MouseStartX = x;
    MouseStartY = y;
}

Vector3<float> GameRender::GetCameraSource(float Dist)
{
    // Create a vector that is rotated about the center
    Vector3<float> CameraSource(cos(CameraRotation), 0.7f + CameraPitchOffset, sin(CameraRotation));
    
    // Grow the vector away
    return CameraTarget + CameraSource * Dist;
}

void GameRender::GetUserSelectionRay(Vector2<int> MousePos, Vector3<float>* SelectionPos, Vector3<float>* SelectionRay)
{
    // Where the camera origin (center) is
    Vector3<float> ViewOrigin = GetCameraSource(GameRender_MinZoom);
    
    // Compute camera face (for directional vector), left vector, and up vector
    Vector3<float> ViewDirection = CameraTarget - ViewOrigin;
    ViewDirection.Normalize();
    
    // Up is always y+
    Vector3<float> UpViewDirection(0, 1, 0);
    
    // Getting left vector is easy:
    Vector3<float> RightViewDirection = Vector3Cross<float>(ViewDirection, UpViewDirection);
    RightViewDirection.Normalize();
    
    // Get the correct up vector (since the camera may be pitched)
    UpViewDirection = Vector3Cross<float>(RightViewDirection, ViewDirection);
    
    // Compute the world mouse coordinates
    // -SizeWidth / Fovy to SizeWidth / Fovy, -SizeHeight / Fovy to SizeHeight / Fovy
    const float Fovy = MainWindow->GetFovy();
    
    // Apply y-selection change (has to be negative since y+ world is up, and y+ screen is down
    ViewOrigin += UpViewDirection * ((-float(MousePos.y - WindowHeight / 2) / Fovy) * 2.0f);
    
    // Apply x-selection change
    ViewOrigin += RightViewDirection * ((float(MousePos.x - WindowWidth / 2) / Fovy) * 2.0f);
    
    // Post data
    *SelectionPos = ViewOrigin;
    *SelectionRay = ViewDirection;
}
