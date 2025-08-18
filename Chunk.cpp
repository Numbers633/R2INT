#include "Chunk.h"
#include "World.h"
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

void Chunk::FillWithVoidState(char voidState)
{
    for (int y = 0; y < GRID_DIMENSIONS; ++y) {
        for (int x = 0; x < GRID_DIMENSIONS; ++x) {
            Grid[x][y] = voidState;
            OldGrid[x][y] = voidState;
        }
    }
    Fill = GRID_DIMENSIONS * GRID_DIMENSIONS * voidState; // Fill is the total number of filled cells
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
    __int8 NewVoidState = ApplyRules(world.VoidState == 1 ? 33554431 : 0, rules) ? 1 : 0;

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

                    // Assume OldVoidState
                    int state = world.GetCellStateAtOld({ gx, gy }); // <-- Use OldGrid

                    neighborhoodInt += state * weight;
                    weight >>= 1;
                }
            }

            newGrid[x][y] = ApplyRules(neighborhoodInt, rules) ? 1 : 0;
            Fill += newGrid[x][y];

            // Assume NewVoidState
            if(newGrid[x][y] != NewVoidState && (x < 2 || x >= GRID_DIMENSIONS - 2 || y < 2 || y >= GRID_DIMENSIONS - 2))
            {
                //EnsureNeighborsExist(world);
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
    if (!grid.NeedsNeighbors(world.VoidState)) return;

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

// --- 1) Change signature of NeedsNeighbors to accept the void state ---
bool Chunk::NeedsNeighbors(__int8 voidState) const {
    // Check if any cell of the previous-generation (OldGrid) is non-void
    // and within 2 cells of the edge.
    for (int y = 0; y < GRID_DIMENSIONS; ++y) {
        for (int x = 0; x < GRID_DIMENSIONS; ++x) {
            if (OldGrid[x][y] != voidState &&
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
            if (dx == -1) {
                for (int x = 0; x < GRID_DIMENSIONS; x++) {
                    if (Grid[x][0] != world.VoidState) {
                        shouldCreate = true;
                        break;
                    }
                }
            }
            if (dx == 1) {
                for (int x = 0; x < GRID_DIMENSIONS; x++) {
                    if (Grid[x][GRID_DIMENSIONS - 1] != world.VoidState) {
                        shouldCreate = true;
                        break;
                    }
                }
            }

            // Vertical edges
            if (dy == -1) {
                for (int x = 0; x < GRID_DIMENSIONS; x++) {
                    if (Grid[x][0] != world.VoidState) {
                        shouldCreate = true;
                        break;
                    }
                }
            }
            if (dy == 1) {
                for (int x = 0; x < GRID_DIMENSIONS; x++) {
                    if (Grid[x][GRID_DIMENSIONS - 1] != world.VoidState) {
                        shouldCreate = true;
                        break;
                    }
                }
            }

            if (!shouldCreate) continue;

            // Create the neighbor grid in the world
            auto& newGrid = world.contents[neighborCoord];
            newGrid.CoordinateX = nx;
            newGrid.CoordinateY = ny;
            newGrid.Fill = 0;

            newGrid.FillWithVoidState(world.VoidState);

            // Initialize OldGrid from the current grid edge
            for (int i = 0; i < GRID_DIMENSIONS; i++) {
                if (dy == -1 && Grid[i][0] != world.VoidState)
                    newGrid.OldGrid[i][GRID_DIMENSIONS - 1] = Grid[i][0]; // top edge
                if (dy == 1 && Grid[i][GRID_DIMENSIONS - 1] != world.VoidState)
                    newGrid.OldGrid[i][0] = Grid[i][GRID_DIMENSIONS - 1]; // bottom edge
                if (dx == -1 && Grid[0][i] != world.VoidState)
                    newGrid.OldGrid[GRID_DIMENSIONS - 1][i] = Grid[0][i]; // left edge
                if (dx == 1 && Grid[GRID_DIMENSIONS - 1][i] != world.VoidState)
                    newGrid.OldGrid[0][i] = Grid[GRID_DIMENSIONS - 1][i]; // right edge
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
    return false; // The chunks are equal, so return false
}