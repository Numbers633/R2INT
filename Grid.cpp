#include "Grid.h"
#include "Debug.h"
#include <vector>
#include <random>
#include <iostream>

// Create Grid64 at origin
Grid64::Grid64() {
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
Grid64::Grid64(int x, int y) {
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

__int8 Grid64::GetCellStateAt(sf::Vector2i localXY) const
{
    return Grid[localXY.x][localXY.y];
}

void Grid64::Clear()
{
    for (int x = 0; x < GRID_DIMENSIONS; x++) {
        for (int y = 0; y < GRID_DIMENSIONS; y++) {

            Grid[x][y] = 0;
            OldGrid[x][y] = 0;
        }
    }
}

void Grid64::RandomizeRect(sf::Rect<int> RandomizedSection, bool Delete, std::mt19937& gen, std::uniform_int_distribution<int>& number_distribution) {
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

void Grid64::Simulate(const R2INTRules& Rules, World& world) {
    // Temporary buffer for the next state
    std::array<std::array<__int8, GRID_DIMENSIONS>, GRID_DIMENSIONS> nextGrid;

    Fill = 0;

    for (int y = 0; y < GRID_DIMENSIONS; ++y) {
        for (int x = 0; x < GRID_DIMENSIONS; ++x) {
            Neighborhood nh;

            // Build the 5x5 neighborhood
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    int gx = CoordinateX * GRID_DIMENSIONS + x + dx;
                    int gy = CoordinateY * GRID_DIMENSIONS + y + dy;

                    // Convert to grid coords
                    int gridX = (gx >= 0) ? gx / GRID_DIMENSIONS : (gx - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
                    int gridY = (gy >= 0) ? gy / GRID_DIMENSIONS : (gy - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;

                    int localX = (gx % GRID_DIMENSIONS + GRID_DIMENSIONS) % GRID_DIMENSIONS;
                    int localY = (gy % GRID_DIMENSIONS + GRID_DIMENSIONS) % GRID_DIMENSIONS;

                    Grid64* targetGrid = world.GetNeighborGrid(gridX, gridY);
                    int state = targetGrid ? targetGrid->GetCellStateAt({ localX, localY }) : world.VoidState;

                    nh[(dy + 2) * 5 + (dx + 2)] = state;
                }
            }

            // Apply rules
            nextGrid[x][y] = ApplyRules(nh, Rules);
            Fill += nextGrid[x][y];
        }
    }

    // Commit nextGrid to current grid
    for (int y = 0; y < GRID_DIMENSIONS; ++y)
        for (int x = 0; x < GRID_DIMENSIONS; ++x)
            Grid[x][y] = nextGrid[x][y];

    // Copy to OldGrid for next generation
    for (int y = 0; y < GRID_DIMENSIONS; ++y)
        for (int x = 0; x < GRID_DIMENSIONS; ++x)
            OldGrid[x][y] = Grid[x][y];
}

void Grid64::ResetOld()
{
    for (int x = 0; x < 64; x++)
    {
        for (int y = 0; y < 64; y++)
        {
            OldGrid[x][y] = Grid[x][y];
        }
    }
}

void EnsureNeighborsExist(World& world, Grid64& grid) {
    if (!grid.NeedsNeighbors()) return;

    GridCoord origin{ grid.CoordinateX, grid.CoordinateY };

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;

            GridCoord neighborCoord{ origin.x + dx, origin.y + dy };
            if (world.contents.find(neighborCoord) == world.contents.end()) {
                world.contents[neighborCoord] = Grid64(neighborCoord.x, neighborCoord.y);
            }
        }
    }
}

bool Grid64::NeedsNeighbors() const {
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

static bool operator!=(const Grid64& lhs, const Grid64& rhs)
{
    if (lhs.CoordinateX != rhs.CoordinateX) return true;
    if (lhs.CoordinateY != rhs.CoordinateY) return true;
    if (lhs.Fill != rhs.Fill) return true;
    if (memcmp(lhs.Grid, rhs.Grid, sizeof(lhs.Grid)) != 0) return true;
    return false;
}

//
// World class implementation
//
 
World::World() {
    Grid64 initial;
    initial.CoordinateX = 0;
    initial.CoordinateY = 0;
    contents[{0, 0}] = initial;
}

void World::PaintAtCell(sf::Vector2i p, int newState)
{
    int gx = p.x / GRID_DIMENSIONS;
    int gy = p.y / GRID_DIMENSIONS;
    int lx = (p.x % GRID_DIMENSIONS + GRID_DIMENSIONS) % GRID_DIMENSIONS;
    int ly = (p.y % GRID_DIMENSIONS + GRID_DIMENSIONS) % GRID_DIMENSIONS;

    auto& grid = contents[{gx, gy}];
    grid.CoordinateX = gx;
    grid.CoordinateY = gy;

    int oldState = grid.Grid[lx][ly];

    grid.Grid[lx][ly] = newState;
    grid.OldGrid[lx][ly] = newState;

    // Update Fill to reflect live cells
    grid.Fill += (newState - oldState);
}


__int8 World::GetCellStateAt(sf::Vector2i p)
{
    int gx = (p.x >= 0) ? p.x / GRID_DIMENSIONS : (p.x - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
    int gy = (p.y >= 0) ? p.y / GRID_DIMENSIONS : (p.y - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
    int lx = (p.x % GRID_DIMENSIONS + GRID_DIMENSIONS) % GRID_DIMENSIONS;
    int ly = (p.y % GRID_DIMENSIONS + GRID_DIMENSIONS) % GRID_DIMENSIONS;

    GridCoord coord = { gx, gy };

    auto it = contents.find(coord);
    if (it != contents.end())
        return it->second.Grid[lx][ly];

    return 0; // Default state for empty space
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
        Grid64& grid = contents.at(coord);
        EnsureNeighborsExist(*this, grid);
    }

    // Step 2: Remove empties first to prevent dangling pointers
    DeleteEmptyGrids(contents);

    // Step 3: Relink neighbors after changes
    LinkAllNeighbors();

    // Step 4: Simulate all grids safely
    keys.clear();
    for (const auto& [coord, grid] : contents)
        keys.push_back(coord);

    for (const GridCoord& coord : keys) {
        Grid64& grid = contents.at(coord);
        grid.Simulate(Rules, *this);
    }
}


Grid64* World::GetNeighborGrid(int x, int y) {
    GridCoord coord = { x, y };

    // Check if the neighbor grid exists in the map
    auto it = contents.find(coord);
    if (it != contents.end()) {
        return &it->second;  // Return the existing grid
    }

    // If it doesn't exist, create and link the new grid
    Grid64 newGrid(coord.x, coord.y);
    contents[coord] = newGrid;  // Add the new grid to the map

    // Return the newly created grid
    return &contents[coord];
}

void DeleteEmptyGrids(std::unordered_map<GridCoord, Grid64>& worldMap) {
    for (auto it = worldMap.begin(); it != worldMap.end(); ) {
        if (it->second.Fill == 0) {
            it = worldMap.erase(it);
        }
        else {
            ++it;
        }
    }
}