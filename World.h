#pragma once
#include "Chunk.h"
#include <unordered_map>

struct World {
    std::unordered_map<GridCoord, Chunk> contents;
    int Generation = 0;
    int n_states = 2;
    float cellSize = 40.f;
    __int8 VoidState = 0; // Default state for empty space; will be replaced with VoidAgar later
    
    World();

    void Simulate(const R2INTRules& Rules);
    void PaintAtCell(sf::Vector2i p, int newState);
    void LinkAllNeighbors();

    Chunk* GetNeighborGrid(int x, int y);
    __int8 GetCellStateAt(sf::Vector2i p) const; // Uses Grid
    __int8 GetCellStateAtOld(sf::Vector2i p) const; // Uses OldGrid

    void EnsureAllPotentialNeighborGridsExist();

    void Draw(sf::RenderWindow& window, const std::vector<sf::Color>& colors);

    sf::Vector2i GetWorldCoords(const sf::Vector2f& screenPos) const;

    // GetRect function; returns global coordinates
    sf::IntRect GetRect() const;
    void PrintRLE() const;
};

void DeleteEmptyGrids(std::unordered_map<GridCoord, Chunk>& worldMap, __int8 VoidState);
void EnsureNeighborsExist(World& world, Chunk& grid);