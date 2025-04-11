#include "Grid.h"
#include "Debug.h"
#include <vector>
#include <random>
#include <iostream>

const bool SINGLE_GRID = false;

int GetDirFromXY(int x, int y)
{ 
    return y + (x * 3) + 4;
}

int GetYCoordinateFromDir(int dir)
{
    return dir / 3;
}

int GetXCoordinateFromDir(int dir)
{
    return (dir % 3) - 1;
}

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

    // Debug: randomize grid
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd(0, 100);
    sf::Rect<int> rc;
    rc.position = sf::Vector2i(50, 50);
    rc.size = sf::Vector2i(101, 101);
    RandomizeRect(rc, false, gen, rnd);
}

void Grid64::Reset()
{
    for (int y = 0; y < GRID_DIMENSIONS; y++) {
        for (int x = 0; x < GRID_DIMENSIONS; x++) {

            Grid[y][x] = 0;
            OldGrid[y][x] = 0;
        }
    }
}

void Grid64::RandomizeRect(sf::Rect<int> RandomizedSection, bool Delete, std::mt19937& gen, std::uniform_int_distribution<int>& number_distribution) {
    for (int y = 0; y < GRID_DIMENSIONS; y++) {
        for (int x = 0; x < GRID_DIMENSIONS; x++) {
            sf::Vector2i cellPoint(y, x);  // Equivalent to POINT {x, y}

            // Check if the point is within the randomized section
            if (!RandomizedSection.contains(cellPoint)) {
                if (Delete) {
                    Grid[y][x] = 0;
                    OldGrid[y][x] = 0;
                }

                continue;
            }

            int n = number_distribution(gen);
            Grid[y][x] = n / 51; // Bugged for higher Fill percentages
            OldGrid[y][x] = Grid[y][x];
        }
    }
}

void Grid64::Simulate(const R2INTRules &Rules) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> number_distribution(0, 100);

    Fill = 0;

    for (int y = 0; y < GRID_DIMENSIONS; y++) {
        for (int x = 0; x < GRID_DIMENSIONS; x++) {
            int NeighborhoodWeight = 16777216;
            int R2INT = 0;
            bool Void = false;

            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    int CellState = 0;
                    if (nx >= 0 && nx < GRID_DIMENSIONS && ny >= 0 && ny < GRID_DIMENSIONS) {
                        CellState = OldGrid[nx][ny];
                    }
                    R2INT += CellState * NeighborhoodWeight;
                    NeighborhoodWeight /= 2;
                }
            }

            Grid[x][y] = ApplyRules(R2INT, Rules);
            Fill += Grid[x][y]; // Potentially bugged for multistate patterns
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

static bool operator!=(Grid64 lhs, Grid64 rhs)
{
    if (lhs.CoordinateX != rhs.CoordinateX)
        return false;
    if (lhs.CoordinateY != rhs.CoordinateY)
        return false;
    if (lhs.Fill != rhs.Fill)
        return false;
    if (lhs.Grid != rhs.Grid)
        return false;
    return true;
}