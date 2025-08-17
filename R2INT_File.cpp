#include <fstream>
#include <iostream>
#include <random>
#include "OffsetStruct.h"
#include "R2INT_File.h"

void SaveTor2intFile(R2INTRules& saveRule)
{
    std::cout << "Enter the filename to save your rule to: ";
    std::string saveName = "";
    std::cin >> saveName;

    // Append extension if not already present
    if (saveName.size() < 6 || saveName.substr(saveName.size() - 6) != ".r2int")
    {
        saveName += ".r2int";
    }

    std::ofstream outFile(saveName);
    if (!outFile)
    {
        std::cerr << "Error: Could not open " << saveName << " for writing.\n";
        return;
    }

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

            // Instead of cout, write to the file
            outFile << n[12] << n[7] << n[8] << n[13] << n[18] << n[17] << n[16] << n[11] << n[6];
            outFile << n[2] << n[3] << n[4] << n[9] << n[14] << n[19] << n[24] << n[23] << n[22];
            outFile << n[21] << n[20] << n[15] << n[10] << n[5] << n[0] << "\n";
        }
    }

    outFile.close();
    std::cout << "Save complete!" << std::endl;
}
