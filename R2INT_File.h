#pragma once

#include <fstream>
#include <iostream>
#include "OffsetStruct.h"

std::string MergeLinesWithX(const std::vector<std::string>& lines);
void SaveTor2intFile(R2INTRules& saveRule);