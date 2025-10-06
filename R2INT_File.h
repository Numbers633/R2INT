#pragma once

#include <fstream>
#include <iostream>
#include "OffsetStruct.h"

void SaveTor2intFile(R2INTRules& saveRule);
void LoadFromr2intFile(R2INTRules& loadRule);