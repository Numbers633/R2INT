#include <fstream>
#include <iostream>
#include <random>
#include <string>
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

            outFile << n[12] << n[7] << n[8] << n[13] << n[18] << n[17] << n[16] << n[11] << 
                n[6] << n[2] << n[3] << n[4] << n[9] << n[14] << n[19] << n[24] << n[23] << 
                n[22] << n[21] << n[20] << n[15] << n[10] << n[5] << n[0] << n[1] << std::endl;
        }
    }

    outFile.close();
    std::cout << "Save complete!" << std::endl;
}

void LoadFromr2intFile(R2INTRules& loadRule)
{
    std::cout << "Enter the filename to load your rule from: ";
    std::string loadName = "";
    std::cin >> loadName;
    // Append extension if not already present
    if (loadName.size() < 6 || loadName.substr(loadName.size() - 6) != ".r2int")
    {
        loadName += ".r2int";
    }
    std::ifstream inFile(loadName);
    if (!inFile)
    {
        std::cerr << "Error: Could not open " << loadName << " for reading.\n";
        return;
    }
    std::cout << "Loading from " << loadName << std::endl;
    // Clear existing rule
    for (unsigned int i = 0; i < 33554432; i++)
    {
        loadRule[i] = 0;
    }
    std::string line;
    while (std::getline(inFile, line))
    {
        if (line.length() != 25)
        {
            std::cerr << "Warning: Skipping invalid line (incorrect length): " << line << std::endl;
            continue;
        }
        Neighborhood n;
        const int perm[25] = {
        12, 7, 8, 13, 18, 17, 16, 11, 6, 2,
         3,  4, 9, 14, 19, 24, 23, 22, 21, 20,
        15, 10, 5,  0,  1
        };

        for (int i = 0; i < 25; i++)
        {
            int bit = (line[i] == '1') ? 1 : 0;
            n[perm[i]] = bit;
        }

        if (n[12] > 1)
        {
            std::cerr << "Warning: Skipping invalid line (center cell state > 1): " << line << std::endl;
            continue;
        }

        // Set rule for all symmetric variants
        for (const auto& variant : GetAllSymmetries(n))
        {
            int index = ConvertNeighborhoodToInt(variant);
            loadRule[index] = 1;
        }
    }
    inFile.close();
    std::cout << "Load complete!" << std::endl;
}