#pragma once

#include "resource.h"
#include "Debug.h"
#include "OffsetStruct.h"
#include <vector>

#define SHAPE_SQUARE 128
#define SHAPE_DIAMOND 129
#define SHAPE_OCTAGON 130

#define RS_B0 0
#define RS_NORMAL 1
#define RS_EXPLOSIVELESS 2
#define RS_R1OT 3

#define SYMMETRY_C1    256
#define SYMMETRY_C2_1  257
#define SYMMETRY_C2_2  258
#define SYMMETRY_C2_4  259
#define SYMMETRY_C4_1  260
#define SYMMETRY_C4_4  261
#define SYMMETRY_D2_1  262
#define SYMMETRY_D2_2  263
#define SYMMETRY_D2_X  264
#define SYMMETRY_D4_1  265
#define SYMMETRY_D4_2  266
#define SYMMETRY_D4_4  267
#define SYMMETRY_D4_X1 268
#define SYMMETRY_D4_X4 269
#define SYMMETRY_D8_1  270
#define SYMMETRY_D8_4  271

void InitRule(int RuleStyle, bool ClearRule);
void RandPattern(int Width, int Height, int FillPercent, int Symmetry, int Shape, bool Clear);
void UpdateGameOfLife(const R2INTRules& Conditions);