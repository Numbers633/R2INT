#pragma once
#include <SFML/Graphics.hpp>
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

struct Grid64 {
	int CoordinateX;
	int CoordinateY;
	unsigned __int16 Fill;

	__int8 Grid[GRID_DIMENSIONS][GRID_DIMENSIONS]; // Grid[x][y]
	__int8 OldGrid[GRID_DIMENSIONS][GRID_DIMENSIONS];

	Grid64* neighborGrids[3][3] = {}; // Center = [1][1]

	Grid64(); // empty
	Grid64(int x, int y);

	__int8 GetCellStateAt(sf::Vector2i localXY) const;

	void Clear();
	void RandomizeRect(sf::Rect<int> RandomizedSection, bool Delete, std::mt19937& gen, std::uniform_int_distribution<int>& number_distribution);

	void Simulate(const R2INTRules& Rules);
	void ResetOld();
	
	bool NeedsNeighbors() const;
};

struct World {
	std::unordered_map<GridCoord, Grid64> contents;

	World();

	void Simulate(const R2INTRules& Rules);
	void PaintAtCell(sf::Vector2i p, int newState);
	void LinkAllNeighbors();

	__int8 GetCellStateAt(sf::Vector2i p);
};

void DeleteEmptyGrids(std::unordered_map<GridCoord, Grid64>& worldMap);
void EnsureNeighborsExist(World& world, Grid64& grid);