/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

#include "WorldGenerator.h"

WorldGenerator::WorldGenerator()
{
    // Leave null until world is generated
    Random = NULL;
}

WorldGenerator::~WorldGenerator()
{
    // Release the random number gen.
    delete Random;
}

void WorldGenerator::Generate(WorldContainer* WorldData, const char* Seed)
{
    /*** Initialization ***/
    
    // Save the world data reference
    this->WorldData = WorldData;
    
    // Allocate seed or re-seed if needed
    if(Random != NULL)
        delete Random;
    Random = new UtilRand(Seed);
    
    // Generate a world..
    // World data has already been allocated and is, by default, filled with air
    const int WorldWidth = WorldData->GetWorldWidth();
    const int WorldDepth = WorldData->GetWorldHeight();
    
    /*** Generate voronoi graph ***/
    
    // Generations and map size
	Voronoi* v = new Voronoi();
	Vertices* ver = new Vertices();
	const float MapWidth = 5000; // Critical note: If grid increases, so much map width!
    const int GridCount = 50;
    
    // What is the size of each sub-grid
    const float GridSize = MapWidth / GridCount;
    
    // These points are pseudo-random: they are all on the grid + or - that sub-grid's volume
	for(int y = 0; y < GridCount; y++)
    for(int x = 0; x < GridCount; x++)
    {
        float x1 = GridSize * x + (double)Random->Rand()/(double)UINT_MAX * GridSize;
        float y1 = GridSize * y + (double)Random->Rand()/(double)UINT_MAX * GridSize;
        ver->push_back(new VPoint(x1, y1));
	}
    
    // Generate full graph
	Edges* edg = v->GetEdges(ver, MapWidth, MapWidth);
	
    // For each edge
	for(vor::Edges::iterator i = edg->begin(); i != edg->end(); ++i)
	{
        // Missing edge start or end
        if((*i)->start == 0  || (*i)->end == 0)
            UtilAbort("This edge either has an invalid start or end!");
	}
    
    // Draw the diagram to EasyBMP for TESTING
    BMP Image;
    Image.SetSize((int)MapWidth, (int)MapWidth);
    
    // Define a dozen colors helpful to us
    // Black lines
    RGBApixel BlackColor;
    BlackColor.Red = 0;
    BlackColor.Green = 0;
    BlackColor.Blue = 0;
    
    // Red dot
    RGBApixel RedColor;
    RedColor.Red = 255;
    RedColor.Green = 0;
    RedColor.Blue = 0;
    
    // Water
    RGBApixel BlueColor;
    BlueColor.Red = 0;
    BlueColor.Green = 0;
    BlueColor.Blue = 255;
    
    // Ground
    RGBApixel BrownColor;
    BrownColor.Red = 115;
    BrownColor.Green = 61;
    BrownColor.Blue = 26;
    
    // Draw each line, then vertex
	for(vor::Edges::iterator i = edg->begin(); i!= edg->end(); ++i)
	{
		int x1 = (*i)->start->x, y1 = (*i)->start->y;
		int x2 = (*i)->end->x, y2 = (*i)->end->y;
        
        // Draw line
        DrawLine(Image, x1, y1, x2, y2, BlackColor);
	}
    
	for(vor::Vertices::iterator i = ver->begin(); i!= ver->end(); ++i)
	{
        int x1 = (*i)->x;
        int y1 = (*i)->y;
        DrawLine(Image, x1, y1, x1, y1, RedColor);
	}
    
    // Save image to file
    Image.WriteToFile("Pass1.bmp");
    
    /*** Generate Island Volume ***/
    
    // Create a perlin noise to define water
    PerlinNoise OceanMask(4, 8, 1, Seed);
    
    // For each point, determine if it is ocean or land
    int VertexCount = 0;
	for(vor::Vertices::iterator i = ver->begin(); i!= ver->end(); ++i)
	{
        // Normalized position
        int x1 = (*i)->x;
        int y1 = (*i)->y;
        float nx = (float)x1 / MapWidth;
        float ny = (float)y1 / MapWidth;
        
        // Distance from center (normalized)
        float dx = MapWidth / 2 - x1;
        float dy = MapWidth / 2 - y1;
        float Distance = sqrt(dx * dx + dy * dy) / sqrt(pow(MapWidth / 2.0f, 2.0f) + pow(MapWidth / 2.0f, 2.0f));
        
        // Find the probabilty of water: perlin noise + (street) distance from center, normalized
        float WaterProbability = OceanMask.GetNoise2D(nx, ny) - Distance;
        
        // Special rule: edge vertices are *always* water
        if(VertexCount / GridCount == 0 || VertexCount / GridCount == (GridCount - 1) || // Horizontal top and bottom
           VertexCount % GridCount == 0 || VertexCount % GridCount == (GridCount - 1))   // Vertical left and right
            FloodFill(Image, x1, y1, BlueColor);
        // Probability of water
        else if(WaterProbability < 0.35f)
            FloodFill(Image, x1, y1, BlueColor);
        else
            FloodFill(Image, x1, y1, BrownColor);
        
        // Grow vertex count
        VertexCount++;
	}
    
    // Save image to file
    Image.WriteToFile("Pass2.bmp");
    
    /*** Generate Height Map ***/
    
    // Default the land color to dark green, and the old land color to water (initial shore-land state)
    RGBApixel LandColor;
    LandColor.Red = 0;
    LandColor.Green = 100;
    LandColor.Blue = 0;
    
    RGBApixel OldLandColor = BlueColor;
    
    // Keep repeating until we have no shore lines
    int HeightStep = 0;
    while(true)
    {
        // Queue of all shore-line tiles
        printf("Itteration: %d\n", HeightStep);
        Queue< VPoint* > ShoreTiles;
        
        // Find all coastline edges
        for(vor::Edges::iterator i = edg->begin(); i!= edg->end(); ++i)
        {
            // For each edge, see if is a coast-line
            if(IsColor(Image, (*i)->left, OldLandColor) && IsColor(Image, (*i)->right, BrownColor))
            {
                // Save (right) land tile to visit
                ShoreTiles.Enqueue((*i)->right);
            }
            else if(IsColor(Image, (*i)->right, OldLandColor) && IsColor(Image, (*i)->left, BrownColor))
            {
                // Save (left) land tile to visit
                ShoreTiles.Enqueue((*i)->left);
            }
        }
        
        // Give up if we have no more shore tiles to process
        if(ShoreTiles.IsEmpty())
            break;
        
        // Replace with red color
        while(!ShoreTiles.IsEmpty())
        {
            VPoint* Pt = ShoreTiles.Dequeue();
            FloodFill(Image, Pt->x, Pt->y, LandColor);
        }
        
        // Reset for the next color
        OldLandColor = LandColor;
        LandColor.Red += 20;
        LandColor.Green += 20;
        LandColor.Blue += 20;
        
        HeightStep++;
    }
    
    // Save image to file
    Image.WriteToFile("Pass3.bmp");
    
    /*** Image to Model ***/
    
    // Define the sea-level (from ground: 0)
    const int SeaLevel = WorldDepth * (2.0f / 3.0f);
    
    // For each block-column
    for(int z = 0; z < WorldWidth; z++)
    for(int x = 0; x < WorldWidth; x++)
    {
        // Map from game-world coordinates to generated map
        int ImageX = ((float)z / (float)WorldWidth) * MapWidth;
        int ImageY = ((float)x / (float)WorldWidth) * MapWidth;
        RGBApixel Color = Image.GetPixel(ImageX, ImageY);
        
        // If this is blue (or black for now), it's water
        if((Color.Red == BlueColor.Red && Color.Green == BlueColor.Green && Color.Blue == BlueColor.Blue) ||
           (Color.Red == 0 && Color.Green == 0 && Color.Blue == 0))
        {
            for(int y = 0; y < SeaLevel; y++)
                WorldData->SetBlock(x, y, z, dBlockType_Water);
        }
        // Else, is ground, convert into a height map
        else
        {
            // Lowest height is G = 100, then G += 20 for each HeightStep
            float NHeight = ((float)(Color.Green - 100) / 20.0f) / (float)(max(1, HeightStep - 1));
            printf("%f\n", NHeight);
            
            // Apply x^2 on this normalized height
            NHeight = SeaLevel + 1 + pow(NHeight, 2.0f) * WorldDepth * (1.0f / 3.0f);
            
            // Fill from bottom to top
            for(int y = 0; y < (int)NHeight; y++)
                WorldData->SetBlock(x, y, z, dBlockType_Stone);
        }
    }
}

void WorldGenerator::FloodFill(BMP& Image, int x, int y, RGBApixel Color)
{
    // Fill this initial pixel with the color
    RGBApixel OriginalColor = Image.GetPixel(x, y);
    
    // Create a queue for all adjacent pixels, start at initial
    Queue< Vector2<int> > Next;
    Next.Enqueue(Vector2<int>(x, y));
    
    // Keep filling until the queue is empty
    while(!Next.IsEmpty())
    {
        Vector2<int> Pos = Next.Dequeue();
        
        // Bounds check
        if(Pos.x < 0 || Pos.y < 0 || Pos.x >= Image.TellWidth() || Pos.y >= Image.TellWidth())
            continue;
        
        // Get the color, should be same as the original pixel
        RGBApixel NextColor = Image.GetPixel(Pos.x, Pos.y);
        if(NextColor.Red != OriginalColor.Red || NextColor.Green != OriginalColor.Green || NextColor.Blue != OriginalColor.Blue)
            continue;
        
        // Don't fill this color if it is already the target color
        if(NextColor.Red == Color.Red && NextColor.Green == Color.Green && NextColor.Blue == Color.Blue)
            continue;
        
        // Good to fill, and enqueue all the next pixels
        Image.SetPixel(Pos.x, Pos.y, Color);
        Next.Enqueue(Vector2<int>(Pos.x+1, Pos.y));
        Next.Enqueue(Vector2<int>(Pos.x-1, Pos.y));
        Next.Enqueue(Vector2<int>(Pos.x, Pos.y+1));
        Next.Enqueue(Vector2<int>(Pos.x, Pos.y-1));
    }
}

bool WorldGenerator::IsColor(BMP& Image, VPoint* Point, RGBApixel TargetColor)
{
    RGBApixel Color = Image.GetPixel(Point->x, Point->y);
    if(Color.Red == TargetColor.Red && Color.Green == TargetColor.Green && Color.Blue == TargetColor.Blue)
        return true;
    else
        return false;
}
