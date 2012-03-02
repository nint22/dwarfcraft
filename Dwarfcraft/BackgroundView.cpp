/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "BackgroundView.h"

BackgroundView::BackgroundView(GrfxObject* Parent, int ObjectDepth)
    : GrfxObject(Parent, ObjectDepth)
{
    // Start in the morning
    TotalTime = 6000.0f;
    
    // Load the sun and moon images
    SunTextureID = g2LoadImage("sun.png", NULL, NULL, NULL, false, false);
    MoonTextureID = g2LoadImage("moon.png", NULL, NULL, NULL, false, false);
}

BackgroundView::~BackgroundView()
{
    
}

void BackgroundView::Render()
{
    /*** Colored Background ***/
    
    // Go into a 2D mode to draw the background
    Set2D(true);
    glLoadIdentity();
    
    // Get the top and bottom sky colors
    Vector3<float> BottomColor, TopColor;
    GetSkyColorPair(TotalTime, &BottomColor, &TopColor);
    
    // Disable depth
    glDisable(GL_DEPTH_TEST);
    
    // Draw quad (filled)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    glColor3f(TopColor.x, TopColor.y, TopColor.z);
    glVertex2f(0.0f, 0.0f);
    
    glColor3f(BottomColor.x, BottomColor.y, BottomColor.z);
    glVertex2f(0.0f, (float)WindowHeight);
    
    glColor3f(BottomColor.x, BottomColor.y, BottomColor.z);
    glVertex2f((float)WindowWidth, (float)WindowHeight);
    
    glColor3f(TopColor.x, TopColor.y, TopColor.z);
    glVertex2f((float)WindowWidth, 0.0f);
    glEnd();
    
    /*** Sun, Moon, Stars ***/
    
    // Draw sun moving across the sky during the day
    float SunProgress = (BackgroundView_TimeRanges[1][1] - TotalTime) / (BackgroundView_TimeRanges[1][1] - BackgroundView_TimeRanges[1][0]);
    
    // If within [0, 1],it should be on-creen
    if(SunProgress >= 0 && SunProgress <= 1)
        RenderScreenTexture(WindowWidth / 2 + (WindowWidth / 3) * cos(SunProgress * UtilPI), WindowHeight / 2 - (WindowHeight / 2) * sin(SunProgress * UtilPI), 64, 64, SunTextureID);
    
    // Draw moon moving across the sky during the night
    float MoonProgress = (BackgroundView_TimeRanges[3][1] - TotalTime) / (BackgroundView_TimeRanges[3][1] - BackgroundView_TimeRanges[3][0]);
    
    // If within [0, 1],it should be on-creen
    if(MoonProgress >= 0 && MoonProgress <= 1)
        RenderScreenTexture(WindowWidth / 2 + (WindowWidth / 3) * cos(MoonProgress * UtilPI), WindowHeight / 2 - (WindowHeight / 2) * sin(MoonProgress * UtilPI), 64, 64, MoonTextureID);
    
    /*** Done ***/
    
    // Go into a 3D mode
    Set2D(false);
    glLoadIdentity();
    
    // Eable depth
    glEnable(GL_DEPTH_TEST);
}

void BackgroundView::Update(float dT)
{
    // Convert to minecraft time (sped up for debugging)
    TotalTime += dT * 10.0f;
    
    // Cap down as needed
    if(TotalTime >= 24000.0f)
        TotalTime = 0.0f;
}

void BackgroundView::WindowResizeEvent(int NewWidth, int NewHeight)
{
    WindowWidth = NewWidth;
    WindowHeight = NewHeight;
}

void BackgroundView::RenderScreenTexture(int x, int y, int width, int height, GLuint TextureID)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0, 0); glColor3f(1, 1, 1); glVertex3f(x + 0, y + 0, 0.0);
        glTexCoord2f(1, 0); glColor3f(1, 1, 1); glVertex3f(x + 0, y + height, 0.0);
        glTexCoord2f(1, 1); glColor3f(1, 1, 1); glVertex3f(x + width, y + height, 0.0);
        glTexCoord2f(0, 1); glColor3f(1, 1, 1); glVertex3f(x + width, y + 0, 0.0);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void BackgroundView::GetSkyColorPair(float Time, Vector3<float>* BottomColor, Vector3<float>* TopColor)
{
    // Total amount of time we look in the past and future to merge colors with
    static const float MergeRange = 3000;
    
    // Create two colors, the bottom being in the past so we have a
    // smooth change in colors
    Vector3<float> Color;
    for(int j = 0; j < 2; j++)
    {
        // Move time back for the bottom piece (j == 1)
        if(j == 1)
            Time -= 2000;
        
        // For each time range
        for(int i = 0; i < 4; i++)
        {
            // Are we in this color's range?
            if(Time >= BackgroundView_TimeRanges[i][0] && Time <= BackgroundView_TimeRanges[i][1])
            {
                // Colors in question
                Vector3<float> PrevColor = BackgroundView_TimeColors[(i == 0) ? 3 : (i - 1)]; // If 0, go back to 3, else i -1
                Vector3<float> CurrentColor = BackgroundView_TimeColors[i];
                
                // Calc intensity
                float Intensity = fmin(fabs((Time - BackgroundView_TimeRanges[i][0]) / MergeRange), 1);
                
                // Merge this color with the next color
                Color = CurrentColor * Intensity + PrevColor * (1.0f - Intensity);
                
                break;
            }
        }
        
        // Save the color
        if(j == 0)
            *TopColor = Color;
        else
            *BottomColor = Color;
    }
    
    // Done
}
