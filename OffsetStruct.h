#pragma once

#include <array>
#include <cstdint>
#include <vector>

struct OffsetInfo {
	int CellXOffset;
	int CellYOffset;
	double PixelXOffset; // 0-1 to offset between cells
	double PixelYOffset;
};

struct SymmetryInfo {
	int Shape;
	int Symmetry;
	int Fill;
};

struct Neighborhood {
	void operator=(const Neighborhood& rhs) {
		for (int i = 0; i < _countof(ArrayStorage); i++)
			ArrayStorage[i] = rhs.ArrayStorage[i];
	}

	short int& operator[](int Index) {
		return ArrayStorage[Index];
	}

	short int operator[](int Index) const {
		return ArrayStorage[Index];
	}

	short int ArrayStorage[25];

	int8_t countOuterCells() const;
	int8_t countTotalCells() const;
};

// R2INT data structure
// Data is stored in a non-isotropic table
struct R2INTRules {
	bool R2MAP[33554432] = { false };

	bool& operator[](int Index) {  // Now returns a modifiable reference
		return R2MAP[Index];
	}
};

// Rotation functiond
inline Neighborhood RotateNeighborhoodCCW(const Neighborhood& lhs);
inline Neighborhood RotateNeighborhoodCW(const Neighborhood& lhs);
inline Neighborhood MirrorNeighborhoodHorizontally(const Neighborhood& lhs);
inline Neighborhood MirrorNeighborhoodVertically(const Neighborhood& lhs);
// Conversion functions
int ConvertNeighborhoodToInt(const Neighborhood& EvalVector);
inline Neighborhood ConvertIntToNeighborhood(int EvalNumber);
// Isotropic rule functions
int FindLowestNeighborhoodValue(int EvalNumber);
int FindLowestNeighborhoodValue(Neighborhood EvalNeighborhood);
std::array<int, 8> FindAllIsotropicNeighborhoodValues(int EvalNumber);
std::array<int, 8> FindAllIsotropicNeighborhoodValues(Neighborhood EvalNeighborhood);
// Apply rules
bool ApplyRules(int Transition, const R2INTRules& rules);
bool ApplyRules(Neighborhood Transition, const R2INTRules& rules);