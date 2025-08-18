#include "Grid.h"
#include "Debug.h"
#include <vector>
#include <random>
#include <iostream>

// Create Grid64 at origin
Chunk::Chunk() {
    CoordinateX = 0;
    CoordinateY = 0;
    Fill = 0;
    
    for (int y = 0; y < GRID_DIMENSIONS; ++y) {
        for (int x = 0; x < GRID_DIMENSIONS; ++x) {
            Grid[x][y] = 0;
            OldGrid[x][y] = 0;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            neighborGrids[i][j] = nullptr;
        }
    }

    neighborGrids[1][1] = this;
}

// Create Grid64 at coordinates xy
Chunk::Chunk(int x, int y) {
    CoordinateX = x;
    CoordinateY = y;
    Fill = 0;

    for (int y = 0; y < GRID_DIMENSIONS; ++y) {
        for (int x = 0; x < GRID_DIMENSIONS; ++x) {
            Grid[x][y] = 0;
            OldGrid[x][y] = 0;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            neighborGrids[i][j] = nullptr;
        }
    }

    neighborGrids[1][1] = this;
}

__int8 Chunk::GetCellStateAt(sf::Vector2i localXY) const
{
    return OldGrid[localXY.x][localXY.y];
}

void Chunk::Clear()
{
    for (int x = 0; x < GRID_DIMENSIONS; x++) {
        for (int y = 0; y < GRID_DIMENSIONS; y++) {

            Grid[x][y] = 0;
            OldGrid[x][y] = 0;
        }
    }
}

void Chunk::RandomizeRect(sf::Rect<int> RandomizedSection, bool Delete, std::mt19937& gen, std::uniform_int_distribution<int>& number_distribution) {
    for (int x = 0; x < GRID_DIMENSIONS; x++) {
        for (int y = 0; y < GRID_DIMENSIONS; y++) {
            sf::Vector2i cellPoint(x, y);  // Equivalent to POINT {x, y}

            // Check if the point is within the randomized section
            if (!RandomizedSection.contains(cellPoint)) {
                if (Delete) {
                    Grid[x][y] = 0;
                    OldGrid[x][y] = 0;
                }

                continue;
            }

            int n = number_distribution(gen);
            Grid[x][y] = n / 51; // Bugged for higher Fill percentages
            OldGrid[x][y] = Grid[x][y];
        }
    }
}

void Chunk::Simulate(const R2INTRules& rules, World& world)
{
    Fill = 0;
    std::array<std::array<char, GRID_DIMENSIONS>, GRID_DIMENSIONS> newGrid = {};

    for (int y = 0; y < GRID_DIMENSIONS; y++)
    {
        for (int x = 0; x < GRID_DIMENSIONS; x++)
        {
            int neighborhoodInt = 0;
            int weight = 1 << 24;

            for (int dy = -2; dy <= 2; dy++)
            {
                for (int dx = -2; dx <= 2; dx++)
                {
                    int gx = CoordinateX * GRID_DIMENSIONS + x + dx;
                    int gy = CoordinateY * GRID_DIMENSIONS + y + dy;

                    int state = world.GetCellStateAtOld({ gx, gy }); // <-- Use OldGrid

                    neighborhoodInt += state * weight;
                    weight >>= 1;
                }
            }

            newGrid[x][y] = ApplyRules(neighborhoodInt, rules) ? 1 : 0;
            Fill += newGrid[x][y];

            if(newGrid[x][y] != 0 && (x < 2 || x >= GRID_DIMENSIONS - 2 || y < 2 || y >= GRID_DIMENSIONS - 2))
            {
                EnsureNeighborsExist(world);
            }
        }
    }

    Grid = newGrid;
}


// Synchronize the old grid with the current grid
void Chunk::ResetOld()
{
    OldGrid = Grid;
}

void EnsureNeighborsExist(World& world, Chunk& grid) {
    if (!grid.NeedsNeighbors()) return;

    GridCoord origin{ grid.CoordinateX, grid.CoordinateY };

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;

            GridCoord neighborCoord{ origin.x + dx, origin.y + dy };
            if (world.contents.find(neighborCoord) == world.contents.end()) {
                world.contents[neighborCoord] = Chunk(neighborCoord.x, neighborCoord.y);
            }
        }
    }
}

bool Chunk::NeedsNeighbors() const {
    // Check if any cell is within 2 cells of the edge and is non-zero
    for (int y = 0; y < GRID_DIMENSIONS; ++y) {
        for (int x = 0; x < GRID_DIMENSIONS; ++x) {
            if (OldGrid[x][y] != 0 &&
                (x <= 1 || x >= GRID_DIMENSIONS - 2 ||
                    y <= 1 || y >= GRID_DIMENSIONS - 2)) {
                return true;
            }
        }
    }
    return false;
}

void Chunk::EnsureNeighborsExist(World& world) const
{
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue; // skip self

            int nx = CoordinateX + dx;
            int ny = CoordinateY + dy;
            GridCoord neighborCoord{ nx, ny };

            // Skip if neighbor already exists
            if (world.contents.find(neighborCoord) != world.contents.end())
                continue;

            // Check if a neighbor is needed (any live cell near that edge)
            bool shouldCreate = false;

            // Horizontal edges
            if (dx == -1) { for (int y = 0; y < GRID_DIMENSIONS; y++) if (Grid[0][y] != 0) { shouldCreate = true; break; } }
            if (dx == 1) { for (int y = 0; y < GRID_DIMENSIONS; y++) if (Grid[GRID_DIMENSIONS - 1][y] != 0) { shouldCreate = true; break; } }

            // Vertical edges
            if (dy == -1) { for (int x = 0; x < GRID_DIMENSIONS; x++) if (Grid[x][0] != 0) { shouldCreate = true; break; } }
            if (dy == 1) { for (int x = 0; x < GRID_DIMENSIONS; x++) if (Grid[x][GRID_DIMENSIONS - 1] != 0) { shouldCreate = true; break; } }

            if (!shouldCreate) continue;

            // Create the neighbor grid in the world
            auto& newGrid = world.contents[neighborCoord];
            newGrid.CoordinateX = nx;
            newGrid.CoordinateY = ny;
            newGrid.Fill = 0;

            // Initialize OldGrid from the current grid edge
            for (int i = 0; i < GRID_DIMENSIONS; i++) {
                if (dy == -1) newGrid.OldGrid[i][GRID_DIMENSIONS - 1] = Grid[i][0];      // top
                if (dy == 1)  newGrid.OldGrid[i][0] = Grid[i][GRID_DIMENSIONS - 1]; // bottom
                if (dx == -1) newGrid.OldGrid[GRID_DIMENSIONS - 1][i] = Grid[0][i];     // left
                if (dx == 1)  newGrid.OldGrid[0][i] = Grid[GRID_DIMENSIONS - 1][i]; // right
            }
        }
    }
}



static bool operator!=(const Chunk& lhs, const Chunk& rhs)
{
    if (lhs.CoordinateX != rhs.CoordinateX) return true;
    if (lhs.CoordinateY != rhs.CoordinateY) return true;
    if (lhs.Fill != rhs.Fill) return true;
    if (lhs.Grid != rhs.Grid) return true;  // std::array supports operator!= recursively
    return false; // The chunks are unequal, so return false
}

//
// World class implementation
//
 
World::World() {
    Chunk initial;
    initial.CoordinateX = 0;
    initial.CoordinateY = 0;
    contents[{0, 0}] = initial;
}

void World::PaintAtCell(sf::Vector2i p, int newState)
{
    // Determine which grid the cell belongs to
    int gx = (p.x >= 0) ? p.x / GRID_DIMENSIONS : (p.x - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
    int gy = (p.y >= 0) ? p.y / GRID_DIMENSIONS : (p.y - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;

    // Local cell coordinates (wrapped into valid range)
    int lx = ((p.x % GRID_DIMENSIONS) + GRID_DIMENSIONS) % GRID_DIMENSIONS;
    int ly = ((p.y % GRID_DIMENSIONS) + GRID_DIMENSIONS) % GRID_DIMENSIONS;

    GridCoord coord = {gx, gy};

    // Get or create the grid
    auto& grid = contents[coord];
    grid.CoordinateX = gx;
    grid.CoordinateY = gy;

    int oldState = grid.Grid[lx][ly];

    // Paint the cell
    grid.Grid[lx][ly] = newState;
    grid.OldGrid[lx][ly] = newState;

    // Update Fill count
    grid.Fill += (newState - oldState);

    // Remove the chunk if it is empty after erasing
    if (newState == 0 && grid.Fill == 0) {
        contents.erase(coord);
    }
}

__int8 World::GetCellStateAt(sf::Vector2i p)
{
    // Determine which grid the cell is in
    int gx = (p.x >= 0) ? p.x / GRID_DIMENSIONS : (p.x - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
    int gy = (p.y >= 0) ? p.y / GRID_DIMENSIONS : (p.y - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;

    // Local cell coordinates within the grid
    int lx = p.x - gx * GRID_DIMENSIONS;
    int ly = p.y - gy * GRID_DIMENSIONS;

    GridCoord coord = { gx, gy };

    auto it = contents.find(coord);
    if (it != contents.end())
        return it->second.Grid[lx][ly];

    // Default background state
    return VoidState;
}

__int8 World::GetCellStateAtOld(sf::Vector2i p)
{
    // Determine which grid the cell is in
    int gx = (p.x >= 0) ? p.x / GRID_DIMENSIONS : (p.x - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
    int gy = (p.y >= 0) ? p.y / GRID_DIMENSIONS : (p.y - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;

    // Local cell coordinates within the grid
    int lx = p.x - gx * GRID_DIMENSIONS;
    int ly = p.y - gy * GRID_DIMENSIONS;

    GridCoord coord = { gx, gy };

    auto it = contents.find(coord);
    if (it != contents.end())
        return it->second.OldGrid[lx][ly];

    // Default background state
    return VoidState;
}

void World::LinkAllNeighbors()
{
    for (auto& [coord, grid] : contents)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                GridCoord neighborCoord = { coord.x + dx, coord.y + dy };

                auto it = contents.find(neighborCoord);
                if (it != contents.end())
                {
                    grid.neighborGrids[dx + 1][dy + 1] = &it->second;
                }
                else
                {
                    grid.neighborGrids[dx + 1][dy + 1] = nullptr;
                }
            }
        }
    }
}

void World::Simulate(const R2INTRules& Rules) {
    // Step 1: Ensure needed neighbors exist
    std::vector<GridCoord> keys;
    keys.reserve(contents.size());
    for (const auto& [coord, grid] : contents)
        keys.push_back(coord);

    for (const GridCoord& coord : keys) {
        Chunk& grid = contents.at(coord);
        EnsureNeighborsExist(*this, grid);
    }

    // Step 3: Relink neighbors after changes
    LinkAllNeighbors();

    // Step 4: Simulate all grids safely
    keys.clear();
    for (const auto& [coord, grid] : contents)
        keys.push_back(coord);

    for (const GridCoord& coord : keys) {
        Chunk& grid = contents.at(coord);
        grid.Simulate(Rules, *this);
    }

    for (const GridCoord& coord : keys) {
        Chunk& grid = contents.at(coord);
        grid.ResetOld();  // Synchronize old grid with current state
    }

    // Save DeleteEmptyGrids for last to prevent issues during simulation
    DeleteEmptyGrids(contents);
}

Chunk* World::GetNeighborGrid(int x, int y) {
    GridCoord coord = { x, y };

    // Check if the neighbor grid exists in the map
    auto it = contents.find(coord);
    if (it != contents.end()) {
        return &it->second;  // Return the existing grid
    }

    // If it doesn't exist, create and link the new grid
    Chunk newGrid(coord.x, coord.y);
    contents[coord] = newGrid;  // Add the new grid to the map

    // Return the newly created grid
    return &contents[coord];
}

void World::EnsureAllPotentialNeighborGridsExist() {
    std::vector<GridCoord> currentKeys;
    for (const auto& [coord, _] : contents)
        currentKeys.push_back(coord);

    for (const auto& coord : currentKeys) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                GridCoord neighborCoord = { coord.x + dx, coord.y + dy };
                if (contents.find(neighborCoord) == contents.end()) {
                    Chunk& newGrid = contents[neighborCoord];
                    newGrid.CoordinateX = neighborCoord.x;
                    newGrid.CoordinateY = neighborCoord.y;
                }
            }
        }
    }
}


void DeleteEmptyGrids(std::unordered_map<GridCoord, Chunk>& worldMap) {
    for (auto it = worldMap.begin(); it != worldMap.end(); ) {
        if (it->second.Fill == 0) {
            it = worldMap.erase(it);
        }
        else {
            ++it;
        }
    }
}