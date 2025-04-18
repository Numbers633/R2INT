#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include "OffsetStruct.h"

#define GRID_DIMENSIONS 64

struct Grid64 {
	int CoordinateX;
	int CoordinateY;
	unsigned __int16 Fill;

	__int8 Grid[GRID_DIMENSIONS][GRID_DIMENSIONS]; // In format Grid[x][y]
	__int8 OldGrid[GRID_DIMENSIONS][GRID_DIMENSIONS];

	Grid64(); // empty

	void Clear();
	void RandomizeRect(sf::Rect<int> RandomizedSection, bool Delete, std::mt19937& gen, std::uniform_int_distribution<int>& number_distribution); // initialize to random

	// Simulation
	void Simulate(const R2INTRules& Rules);
	void ResetOld();
};

struct World {
	std::vector<Grid64> contents;

	World();

	void Simulate(const R2INTRules& Rules);
	void PaintAtCell(sf::Vector2i p, int newState);

	__int8 GetCellStateAt(sf::Vector2i p);
};