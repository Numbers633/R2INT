#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <random>
#include <unordered_map>
#include "OffsetStruct.h"

#define GRID_DIMENSIONS 64

struct GridCoord {
	int x, y;

	bool operator==(const GridCoord& other) const {
		return x == other.x && y == other.y;
	}
};

namespace std {
	template<>
	struct hash<GridCoord> {
		std::size_t operator()(const GridCoord& p) const {
			return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
		}
	};
}

struct World;

struct Chunk {
	int CoordinateX;
	int CoordinateY;
	unsigned __int16 Fill;

    std::array<std::array<__int8, GRID_DIMENSIONS>, GRID_DIMENSIONS> Grid;
    std::array<std::array<__int8, GRID_DIMENSIONS>, GRID_DIMENSIONS> OldGrid;

	Chunk* neighborGrids[3][3] = {}; // Center = [1][1]

	Chunk(); // empty
	Chunk(int x, int y);

	__int8 GetCellStateAt(sf::Vector2i localXY) const;

	void Clear();
    void FillWithVoidState(char voidState);
	void RandomizeRect(sf::Rect<int> RandomizedSection, bool Delete, std::mt19937& gen, std::uniform_int_distribution<int>& number_distribution);

	void Simulate(const R2INTRules& Rules, World& world);
	void ResetOld();

    void EnsureNeighborsExist(World& world) const;
	
	bool NeedsNeighbors(__int8 voidState) const;
};

struct World {
	std::unordered_map<GridCoord, Chunk> contents;

	World();

	void Simulate(const R2INTRules& Rules);
	void PaintAtCell(sf::Vector2i p, int newState);
	void LinkAllNeighbors();

	Chunk* GetNeighborGrid(int x, int y);
	__int8 GetCellStateAt(sf::Vector2i p); // Uses Grid
    __int8 GetCellStateAtOld(sf::Vector2i p); // Uses OldGrid

    void EnsureAllPotentialNeighborGridsExist();

    __int8 VoidState = 0; // Default state for empty space
};

void DeleteEmptyGrids(std::unordered_map<GridCoord, Chunk>& worldMap, __int8 VoidState);
void EnsureNeighborsExist(World& world, Chunk& grid);