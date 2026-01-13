#pragma once
#include "Chunk.h"
#include <unordered_map>

struct World {
    std::unordered_map<GridCoord, Chunk> contents;

    World();

    void Simulate(const R2INTRules& Rules);
    void PaintAtCell(sf::Vector2i p, int newState);
    void LinkAllNeighbors();

    Chunk* GetNeighborGrid(int x, int y);
    __int8 GetCellStateAt(sf::Vector2i p) const; // Uses Grid
    __int8 GetCellStateAtOld(sf::Vector2i p) const; // Uses OldGrid

    void EnsureAllPotentialNeighborGridsExist();

    __int8 VoidState = 0; // Default state for empty space

    void Draw(sf::RenderWindow& window, float cellSize, const std::vector<sf::Color>& colors);

    // GetRect function; returns global coordinates
    sf::IntRect GetRect() const;
    void PrintRLE() const;
};

void DeleteEmptyGrids(std::unordered_map<GridCoord, Chunk>& worldMap, __int8 VoidState);
void EnsureNeighborsExist(World& world, Chunk& grid);