#include <fstream>
#include <iostream>
#include <random>
#include "OffsetStruct.h"
#include "R2INT_File.h"

void SaveTor2intFile(R2INTRules& saveRule)
{
	const int NUM_WORDS = 32;
	std::cout << "Saving rule!" << std::endl;
	std::array<std::string, NUM_WORDS> words = {
		// Core Concepts
		"R2INT", "Rule", "State", "Phase", "Transition", "Cell", "Pattern", "Toroid",

		// Rule Format / Mechanics
		"Ruletable", "Neighborhood", "Moore", "Radius", "Symmetry", "Decay", "Birth", "Survival",

		// Features / UI
		"Editor", "Viewer", "Palette", "Scroll", "Zoom", "Grid", "Cursor", "Step",

		// Pattern Types
		"Oscillator", "StillLife", "Spaceship", "Gun", "Emitter", "Reflector", "Eater", "Glider"
	};
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution rnd(0, NUM_WORDS - 1);

	std::string saveName = "";

	for (int i = 0; i < 3; i++)
	{
		saveName += words[rnd(gen)];
		if (i <= 2)
		{
			saveName += "_";
		}
	}

	saveName += ".r2int";
	
	std::cout << "Saving to " << saveName << std::endl;

	for (int i = 0; i < 33554432; i++)
	{
		if (FindLowestNeighborhoodValue(i) != i)
		{
			continue;
		}
		if (saveRule.R2MAP[i] == true)
		{
			Neighborhood n = ConvertIntToNeighborhood(i);
			std::cout << n[12] << n[7] << n[8] << n[13] << n[18] << n[17] << n[16] << n[11] << n[6];
			std::cout << n[2] << n[3] << n[4] << n[9] << n[14] << n[19] << n[24] << n[23] << n[22];
			std::cout << n[21] << n[20] << n[15] << n[10] << n[5] << n[0] << std::endl;
		}
	}
}