#include "Grid.h"
#include "Debug.h"
#include <vector>
#include <random>
#include <iostream>

Grid64::Grid64() {
    CoordinateX = 0;
    CoordinateY = 0;
    Fill = 0;
    
    for (int y = 0; y < GRID_DIMENSIONS; y++)
    {
        for (int x = 0; x < GRID_DIMENSIONS; x++)
        {
            Grid[x][y] = 0;
            OldGrid[x][y] = 0;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            neighborGrids[i][j] = this; // Later, set this to nullptr by default
        }
    }

    // Debug: randomize grid
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd(0, 100);
    sf::Rect<int> rc;
    rc.position = sf::Vector2i(24, 24);
    rc.size = sf::Vector2i(16, 16);
    RandomizeRect(rc, false, gen, rnd);
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

	for (int y = 0; y < GRID_DIMENSIONS; ++y) {
		for (int x = 0; x < GRID_DIMENSIONS; ++x) {
			int NeighborhoodWeight = 16777216;
			int R2INT = 0;

			for (int dy = -2; dy <= 2; ++dy) {
				for (int dx = -2; dx <= 2; ++dx) {
					int globalX = x + dx;
					int globalY = y + dy;

					int offsetX = (globalX < 0) ? -1 : (globalX >= GRID_DIMENSIONS ? 1 : 0);
					int offsetY = (globalY < 0) ? -1 : (globalY >= GRID_DIMENSIONS ? 1 : 0);

					int localX = (globalX + GRID_DIMENSIONS) % GRID_DIMENSIONS;
					int localY = (globalY + GRID_DIMENSIONS) % GRID_DIMENSIONS;

					Grid64* target = neighborGrids[offsetX + 1][offsetY + 1];
					int state = 0;

					if (target) {
						state = target->OldGrid[localX][localY];
					}

					R2INT += state * NeighborhoodWeight;
					NeighborhoodWeight /= 2;
				}
			}

			Grid[x][y] = ApplyRules(R2INT, Rules);
			Fill += Grid[x][y];
		}
	}

	memcpy(OldGrid, Grid, sizeof(Grid));
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
    contents.resize(1); // Start with one default Grid64
}

void World::Simulate(const R2INTRules& Rules)
{
    for (Grid64& grid : contents)
    {
        grid.Simulate(Rules);
    }
}

void World::PaintAtCell(sf::Vector2i p, int newState)
{
    if (p.x >= 0 && p.x < GRID_DIMENSIONS && p.y >= 0 && p.y < GRID_DIMENSIONS)
    {
        contents[0].Grid[p.x][p.y] = newState;
        contents[0].OldGrid[p.x][p.y] = newState;
    }
}

__int8 World::GetCellStateAt(sf::Vector2i p)
{
    if (p.x >= 0 && p.x < GRID_DIMENSIONS && p.y >= 0 && p.y < GRID_DIMENSIONS)
    {
        return contents[0].Grid[p.x][p.y];
    }

    return 0; // Replace with the current background state;
}