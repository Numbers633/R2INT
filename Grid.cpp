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

void Grid64::Simulate(const R2INTRules& Rules) {
    Fill = 0;

    // Sanity checksum of the OldGrid before simulation (to catch glitches)
    int oldChecksum = 0;
    for (int y = 0; y < GRID_DIMENSIONS; ++y)
        for (int x = 0; x < GRID_DIMENSIONS; ++x)
            oldChecksum += OldGrid[x][y] * (x + 1) * (y + 1);

    for (int y = 0; y < GRID_DIMENSIONS; ++y) {
        for (int x = 0; x < GRID_DIMENSIONS; ++x) {
            int NeighborhoodWeight = 16777216;  // 2^24
            int R2INT = 0;

            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    int globalX = x + dx;
                    int globalY = y + dy;

                    // Determine which neighbor tile this offset is in
                    int offsetX = (globalX < 0) ? -1 : (globalX >= GRID_DIMENSIONS ? 1 : 0);
                    int offsetY = (globalY < 0) ? -1 : (globalY >= GRID_DIMENSIONS ? 1 : 0);

                    int localX = (globalX + GRID_DIMENSIONS) % GRID_DIMENSIONS;
                    int localY = (globalY + GRID_DIMENSIONS) % GRID_DIMENSIONS;

                    Grid64* target = neighborGrids[offsetX + 1][offsetY + 1];
                    int state = 0;

                    if (target != nullptr) {
                        if (localX < 0 || localX >= GRID_DIMENSIONS || localY < 0 || localY >= GRID_DIMENSIONS) {
                            std::cerr << "[ERROR] Invalid index access: (" << localX << "," << localY << ") in grid at ("
                                << target->CoordinateX << "," << target->CoordinateY << ")\n";
                        }
                        else {
                            state = target->OldGrid[localX][localY];
                        }
                    }
                    else {
                        // Assume neighbor is empty if not present
                        state = 0;
                    }

                    R2INT += state * NeighborhoodWeight;
                    NeighborhoodWeight /= 2;
                }
            }

            Grid[x][y] = ApplyRules(R2INT, Rules);
            Fill += Grid[x][y];
        }
    }

    // Copy Grid to OldGrid after updating everything
    for (int y = 0; y < GRID_DIMENSIONS; ++y)
        for (int x = 0; x < GRID_DIMENSIONS; ++x)
            OldGrid[x][y] = Grid[x][y];

    // Sanity checksum after simulation
    int newChecksum = 0;
    for (int y = 0; y < GRID_DIMENSIONS; ++y)
        for (int x = 0; x < GRID_DIMENSIONS; ++x)
            newChecksum += Grid[x][y] * (x + 1) * (y + 1);
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

void World::Simulate(const R2INTRules& Rules) {
    // Step 1: Dynamically add neighbors if needed
    std::vector<GridCoord> keys;
    for (const auto& [coord, grid] : contents)
        keys.push_back(coord); // copy keys to avoid iterator invalidation

    for (const GridCoord& coord : keys) {
        Grid64& grid = contents.at(coord);
        EnsureNeighborsExist(*this, grid);
    }

    // Step 2: Link all neighbors
    LinkAllNeighbors();

    // Step 3: Simulate each grid
    for (auto& [coord, grid] : contents) {
        grid.Simulate(Rules);
    }

    // Step 4: Delete empty grids
    DeleteEmptyGrids(contents);
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
    grid.Grid[lx][ly] = newState;
    grid.OldGrid[lx][ly] = newState;
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