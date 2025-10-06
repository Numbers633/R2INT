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

    short int* begin() { return std::begin(ArrayStorage); }
    short int* end() { return std::end(ArrayStorage); }
    const short int* begin() const { return std::begin(ArrayStorage); }
    const short int* end() const { return std::end(ArrayStorage); }
};

// R2INT rule data structure
// Data is stored in a non-isotropic table
class R2INTRules {
public:
	bool R2MAP[33554432] = { false };
	void ToggleIsotropicTransition(Neighborhood n);

	bool& operator[](int Index) {  // Now returns a modifiable reference
		return R2MAP[Index];
	}

    const bool& operator[](int index) const {
        return R2MAP[index];
    }
};

// Rotation functions
Neighborhood RotateNeighborhoodCCW(const Neighborhood& lhs);
Neighborhood RotateNeighborhoodCW(const Neighborhood& lhs);
Neighborhood MirrorNeighborhoodHorizontally(const Neighborhood& lhs);
Neighborhood MirrorNeighborhoodVertically(const Neighborhood& lhs);
// Shifting functions
Neighborhood ShiftNeighborhood(const Neighborhood& lhs, int dx, int dy);
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