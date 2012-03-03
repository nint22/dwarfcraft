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
    
    // Create array of what the world-top should be filled with
    const int SurfaceTypesCount = 3;
    dBlockType SurfaceTypes[SurfaceTypesCount] = {dBlockType_Dirt, dBlockType_Sand, dBlockType_Dirt};
    
    // Push out some information for debugging
    printf("%d chunks created in a %dx%d world; height: %d\n", (int)pow(WorldWidth / WorldData->GetColumnWidth(), 2.0f), WorldWidth, WorldWidth, WorldDepth);
    
    /*** Perlin Masks ***/
    
    // Just the regular topography
    PerlinNoise* TerrainHeight = new PerlinNoise(WorldWidth, WorldWidth);
    TerrainHeight->Render(float(Random->Rand()) / float(UINT_MAX), 35.0, 0.01);
    
    // Current three biomes: planes (dirt), desert (sand), and forest (trees, noted as dBlockType_Leaves)
    PerlinNoise* TerrainBiomes = new PerlinNoise(WorldWidth, WorldWidth);
    TerrainBiomes->Render(float(Random->Rand()) / float(UINT_MAX), 65.0, 0.01);
    
    /*** Fill bottom with lava ***/
    
    // Fill lower 2 levels with just lava
    for(int z = 0; z < WorldWidth; z += WorldData->GetColumnWidth())
    for(int x = 0; x < WorldWidth; x += WorldData->GetColumnWidth())
        WorldData->FillChunk(x, 0, z, dBlockType_Lava);
    
    /*** Fill each column with stone, dirt, and surface layers ***/
    for(int z = 0; z < WorldWidth; z++)
    for(int x = 0; x < WorldWidth; x++)
    {
        // Print progress
        int Progress = (z * WorldWidth + x);
        if(Progress % 200 == 0)
            printf("\tWorld generation: %.2f%% complete\n", 100.0f * float(Progress + 1) / (WorldWidth * WorldWidth));
        
        // Get the layer heights
        int SurfaceHeight = GetSurfaceHeight(TerrainHeight, x, z);
        int DirtHeight = GetDirtHeight(TerrainHeight, x, z);
        int StoneHeight = GetStoneHeight(TerrainHeight, x, z);
        int Step = (GetSurfaceHeight(TerrainHeight, x, z) - SurfaceHeight) * 2.0f;
        
        // Get the biome type
        int BiomeIndex = (float(TerrainBiomes->GetDepth(x, z)) / 256.0f) * float(SurfaceTypesCount);
        
        // Fill column
        // Note: we are going from bottom-to-up
        for(int y = 1; y < WorldDepth; y++)
        {
            // Get block type
            dBlockType BlockType = dBlockType_Air;
            if(y <= StoneHeight)
                BlockType = dBlockType_RoughStone;
            else if(y <= DirtHeight)
                BlockType = dBlockType_Dirt;
            else if(y <= SurfaceHeight)
                BlockType = SurfaceTypes[BiomeIndex];
            
            // Else, done filling, place some surface items as desired
            else
            {
                // Place foliage in special terrain
                if(BiomeIndex == 0 && (Random->Rand() % 30) == 0)
                    PlaceFoliage(x, z);
                
                // Place tree (only in tree zones)
                else if(BiomeIndex == 2 && (Random->Rand() % 25) == 0)
                    PlaceTree(x, z);
                
                break;
            }
            
            // Make a meta offset for snow, if we are working with dirt and we are high enough
            unsigned char Meta = 0;
            if(BiomeIndex == 0 && y == SurfaceHeight)
                Meta = 1; // Make grass if and only if at the top
            else if(BiomeIndex == 2 && y == SurfaceHeight)
                Meta = ((y > WorldDepth - 20) ? 2 : 1); // Make snow; again only if at top
            
            // Set regular block
            WorldData->SetBlock(x, y, z, dBlock(BlockType, Meta));
            
            // Set special property for top-most block
            if(y == SurfaceHeight)
            {
                dBlock Block(BlockType, Meta);
                if(Step == 0) // Half
                    Block.SetWhole(false);
                else // Full
                    Block.SetWhole(true);
                WorldData->SetBlock(x, y, z, Block);
            }
        }
    }
    
    /*** Place Special Blocks ***/
    
    // Place 100 mineral blobs
    for(int i = 0; i < 100; i++)
        PlaceBlob(Random->Rand() % WorldWidth, Random->Rand() % WorldDepth, Random->Rand() % WorldWidth, dBlockType_Coal);
    
    /*** Optimize Geometry ***/
    
    // If a level is just the same content, coalesce it into a reference with no actual allocations
    WorldData->OptimizeColumns();
    
    // Release perlin masks
    delete TerrainHeight;
    delete TerrainBiomes;
}

void WorldGenerator::PlaceTree(int x, int z)
{
    // Find the surface of this column
    int Surface = WorldData->GetSurfaceDepth(x, z);
    if(!dIsSolid(WorldData->GetBlock(x, Surface, z)))
        return;
    
    // Make it so this block is the root / trunk of the tree
    WorldData->SetBlock(x, Surface, z, dBlockType_Wood);
    
    // We want to place above the surface
    Surface++;
    
    // Randomize tree height
    int Height = 3 + (Random->Rand() % 3);
    
    // Place leaves first (so the trunk overwrites it)
    // Leaf-placement is 2 above the base to 1 above the trunk,
    // all adjacent around the trunk
    for(int y = Surface + 2; y < Surface + Height; y++)
    {
        // Place 3x3 adjacent
        for(int dx = -1; dx <= 1; dx++)
        for(int dz = -1; dz <= 1; dz++)
        {
            if(WorldData->IsWithinWorld(x + dx, y, z + dz))
                WorldData->SetBlock(x + dx, y, z + dz, dBlockType_Leaves);
        }
    }
    
    // Top tip of tree
    if(WorldData->IsWithinWorld(x, Surface + Height, z))
        WorldData->SetBlock(x, Surface + Height, z, dBlockType_Leaves);
    
    // Place trunk
    for(int y = Surface; y < Surface + Height; y++)
        if(WorldData->IsWithinWorld(x, y, z))
            WorldData->SetBlock(x, y, z, dBlockType_Wood);
}

void WorldGenerator::PlaceBlob(int x, int y, int z, dBlockType type, int steps)
{
    // Ignore if done placing, out of bounds, is air, or is above the surface height
    if(steps <= 0 || !WorldData->IsWithinWorld(x, y, z) || WorldData->GetBlock(x, y, z).GetType() == dBlockType_Air)
        return;
    
    // If we can place it, place the block
    if(WorldData->IsWithinWorld(x, y, z))
        WorldData->SetBlock(x, y, z, type);
    
    // Decrease steps and apply to all adjacents (probability 50%)
    for(int dx = -1; dx <= 1; dx++)
    for(int dy = -1; dy <= 1; dy++)
    for(int dz = -1; dz <= 1; dz++)
    {
        // Place on 50% and never at the origin
        if(Random->Rand() % 2 == 0)
            PlaceBlob(x + dx, y + dy, z + dz, type, steps - 1);
    }
}

void WorldGenerator::PlaceFoliage(int x, int z)
{
    // Find the surface of this column
    int Surface = WorldData->GetSurfaceDepth(x, z);
    if(!dIsSolid(WorldData->GetBlock(x, Surface, z)))
        return;
    
    // Move the surface up so we are placing on it, not as it
    Surface++;
    if(Surface >= WorldData->GetWorldHeight())
        return;
    
    // Can be either dBlockType_Grass, dBlockType_Bush, dBlockType_Mushroom
    dBlockType BlockType = dBlockType(dBlockType_Grass + (Random->Rand() % 3));
    int Meta = 0;
    
    // If we are grass, there are 7 forms of it, randomly choose one
    if(BlockType == dBlockType_Grass)
        Meta = Random->Rand() % 7;
    else if(BlockType == dBlockType_Mushroom)
        Meta = Random->Rand() % 2;
    
    // Place the block
    WorldData->SetBlock(x, Surface, z, dBlock(BlockType, Meta));
}

float WorldGenerator::GetSurfaceHeight(PerlinNoise* TerrainHeight, int x, int z)
{
    float SurfaceHeight = float(WorldData->GetWorldHeight() / 6.0f);
    return fmin(WorldData->GetWorldHeight() - 1, (float(TerrainHeight->GetDepth(x, z)) / 255.0f) * SurfaceHeight + WorldData->GetWorldHeight() - SurfaceHeight - 1);
}

float WorldGenerator::GetDirtHeight(PerlinNoise* TerrainHeight, int x, int z)
{
    float SurfaceHeight = float(WorldData->GetWorldHeight() / 8.0f);
    return fmin(WorldData->GetWorldHeight() - 1, (float(TerrainHeight->GetDepth(x, z)) / 255.0f) * SurfaceHeight + WorldData->GetWorldHeight() - SurfaceHeight - float(WorldData->GetWorldHeight() / 6.0f) - 1);
}

float WorldGenerator::GetStoneHeight(PerlinNoise* TerrainHeight, int x, int z)
{
    
    float SurfaceHeight = float(WorldData->GetWorldHeight() / 4.0f);
    return fmin(WorldData->GetWorldHeight() - 1, (float(TerrainHeight->GetDepth(x, z)) / 255.0f) * SurfaceHeight + WorldData->GetWorldHeight() - SurfaceHeight - float(WorldData->GetWorldHeight() / 6.0f) - float(WorldData->GetWorldHeight() / 4) - 1);
}
