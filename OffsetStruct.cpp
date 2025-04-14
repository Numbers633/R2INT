#include "OffsetStruct.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>

//
// Code to manipulate Neighborhoods
//
inline Neighborhood RotateNeighborhoodCCW(const Neighborhood& lhs)
{
	Neighborhood ret = {
		lhs[4], lhs[9], lhs[14], lhs[19], lhs[24],
		lhs[3], lhs[8], lhs[13], lhs[18], lhs[23],
		lhs[2], lhs[7], lhs[12], lhs[17], lhs[22],
		lhs[1], lhs[6], lhs[11], lhs[16], lhs[21],
		lhs[0], lhs[5], lhs[10], lhs[15], lhs[20]
	};

	return ret;
}

inline Neighborhood RotateNeighborhoodCW(const Neighborhood& lhs)
{
	Neighborhood ret = {
		lhs[20], lhs[15], lhs[10], lhs[5], lhs[0],
		lhs[21], lhs[16], lhs[11], lhs[6], lhs[1],
		lhs[22], lhs[17], lhs[12], lhs[7], lhs[2],
		lhs[23], lhs[18], lhs[13], lhs[8], lhs[3],
		lhs[24], lhs[19], lhs[14], lhs[9], lhs[4]
	};

	return ret;
}

inline Neighborhood MirrorNeighborhoodHorizontally(const Neighborhood& lhs)
{
	Neighborhood ret = {
		lhs[4],  lhs[3],  lhs[2],  lhs[1],  lhs[0],
		lhs[9],  lhs[8],  lhs[7],  lhs[6],  lhs[5],
		lhs[14], lhs[13], lhs[12], lhs[11], lhs[10],
		lhs[19], lhs[18], lhs[17], lhs[16], lhs[15],
		lhs[24], lhs[23], lhs[22], lhs[21], lhs[20]
	};

	return ret;
}

inline Neighborhood MirrorNeighborhoodVertically(const Neighborhood& lhs)
{
	Neighborhood ret = {
		lhs[20], lhs[21], lhs[22], lhs[23], lhs[24],
		lhs[15], lhs[16], lhs[17], lhs[18], lhs[19],
		lhs[10], lhs[11], lhs[12], lhs[13], lhs[14],
		lhs[5],  lhs[6],  lhs[7],  lhs[8],  lhs[9],
		lhs[0],  lhs[1],  lhs[2],  lhs[3],  lhs[4]
	};

	return ret;
}
//
// Shift the neighborhood, useful for creating rules
//

Neighborhood ShiftNeighborhood(const Neighborhood& lhs, int dx, int dy)
{
	Neighborhood ret{};
	for (int y = 0; y < 5; ++y)
	{
		for (int x = 0; x < 5; ++x)
		{
			int fromX = x - dx;
			int fromY = y - dy;
			int fromIndex = fromX + 5 * fromY;
			int toIndex = x + 5 * y;
			if (fromX < 0 || fromX > 4 || fromY < 0 || fromY > 4)
			{
				ret[toIndex] = 0;
				continue;
			}
			ret[toIndex] = lhs[fromIndex];
		}
	}
	return ret;
}

//
// Code to convert between R2INT neighborhood and integer data
//
int ConvertNeighborhoodToInt(const Neighborhood& EvalVector)
{
	int EvalResult = 0;
	int NeighborhoodWeight = 16777216;
	int id = 0;

	while (NeighborhoodWeight >= 1)
	{
		EvalResult += EvalVector[id] * NeighborhoodWeight;

		NeighborhoodWeight /= 2;
		id++;
	}

	return EvalResult;
}

Neighborhood ConvertIntToNeighborhood(int EvalNumber)
{
    Neighborhood EvalResult;
    for (int i = 0; i < 25; i++)
    {
        EvalResult[i] = (EvalNumber >> (24 - i)) & 1;  // Direct bit extraction
    }
    return EvalResult;
}

inline Neighborhood GetNeighborhoodFromInt(int EvalTransition)
{
	Neighborhood EvalResult{};
	int NeighborhoodWeight = 16777216; // half of the neighborhood possibilities

	NeighborhoodWeight = 16777216;

	for (int i = 0; i < 25; i++) {
		if (EvalTransition >= NeighborhoodWeight)
		{
			EvalTransition -= NeighborhoodWeight;
			EvalResult[i] = 1;
		}

		NeighborhoodWeight /= 2;
	}

	return EvalResult;
}

// Neighbor counting functions
int8_t Neighborhood::countOuterCells() const
{
	int8_t neighborCount = 0;
	for (int i = 0; i < 25; i++)
	{
		if (i == 12)
			continue;

		neighborCount += ArrayStorage[i];
	}

	return neighborCount;
}
int8_t Neighborhood::countTotalCells() const
{
	int8_t neighborCount = 0;
	for (int i = 0; i < 25; i++)
	{
		neighborCount += ArrayStorage[i];
	}

	return neighborCount;
}

// Isotropic counting functions
int FindLowestNeighborhoodValue(Neighborhood EvalNeighborhood)
{
	int SmallestNumber = ConvertNeighborhoodToInt(EvalNeighborhood);
	Neighborhood ReduceTest = EvalNeighborhood;

	for (int i = 0; i < 4; i++)
	{
		ReduceTest = RotateNeighborhoodCCW(ReduceTest);

		int CompareResult = ConvertNeighborhoodToInt(ReduceTest);
		if (SmallestNumber > CompareResult)
			SmallestNumber = CompareResult;
	}

	ReduceTest = MirrorNeighborhoodHorizontally(EvalNeighborhood);

	for (int i = 0; i < 4; i++)
	{
		ReduceTest = RotateNeighborhoodCCW(ReduceTest);

		int CompareResult = ConvertNeighborhoodToInt(ReduceTest);
		if (SmallestNumber > CompareResult)
			SmallestNumber = CompareResult;
	}

	return SmallestNumber;
}

int FindLowestNeighborhoodValue(int EvalNumber)
{
	Neighborhood EvalResult = ConvertIntToNeighborhood(EvalNumber);

	return FindLowestNeighborhoodValue(EvalResult);
}

std::array<int, 8> FindAllIsotropicNeighborhoodValues(Neighborhood EvalNeighborhood)
{
	std::array<int, 8> ret = {};
	Neighborhood IsotropicTest = EvalNeighborhood;

	for (int i = 0; i < 4; i++)
	{
		ret[i] = ConvertNeighborhoodToInt(IsotropicTest);
		IsotropicTest = RotateNeighborhoodCCW(IsotropicTest);
	}

	IsotropicTest = MirrorNeighborhoodHorizontally(EvalNeighborhood);

	for (int i = 0; i < 4; i++)
	{
		ret[i+4] = ConvertNeighborhoodToInt(IsotropicTest);
		IsotropicTest = RotateNeighborhoodCCW(IsotropicTest);
	}

	return ret;
}

std::array<int, 8> FindAllIsotropicNeighborhoodValues(int EvalNumber)
{
	Neighborhood EvalResult = ConvertIntToNeighborhood(EvalNumber);

	return FindAllIsotropicNeighborhoodValues(EvalResult);
}

bool ApplyRules(int Transition, const R2INTRules& rules) {
	if (Transition < 0 || Transition >= 33554432)
	{
		return false; // Modify if the CA becomes bugged
	}
	return rules.R2MAP[Transition];
}

bool ApplyRules(Neighborhood Transition, const R2INTRules& rules) {
	return ApplyRules(ConvertNeighborhoodToInt(Transition), rules);
}

void R2INTRules::ToggleIsotropicTransition(Neighborhood n)
{
	int newTransition = 1 - R2MAP[ConvertNeighborhoodToInt(n)];
	for (int i = 0; i < 4; i++)
	{
		int t = ConvertNeighborhoodToInt(n);
		R2MAP[t] = newTransition;
		n = RotateNeighborhoodCW(n);
	}

	n = MirrorNeighborhoodVertically(n);

	for (int i = 0; i < 4; i++)
	{
		int t = ConvertNeighborhoodToInt(n);
		R2MAP[t] = newTransition;
		n = RotateNeighborhoodCW(n);
	}
}