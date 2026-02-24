#include <fstream>
#include <iostream>
#include <random>
#include <unordered_set>
#include <string>
#include <vector>
#include <thread>
#include <execution>
#include "OffsetStruct.h"
#include "R2INT_File.h"

void SaveTor2intFile(R2INTRules& saveRule)
{
    std::cout << "Enter the filename to save your rule to: ";
    std::string saveName;
    std::cin >> saveName;

    if (saveName.size() < 6 || saveName.substr(saveName.size() - 6) != ".r2int")
        saveName += ".r2int";

    std::ofstream outFile(saveName);
    if (!outFile) {
        std::cerr << "Error: Could not open " << saveName << " for writing.\n";
        return;
    }

    std::cout << "Saving to " << saveName << std::endl;

    // Dedup set
    std::unordered_set<std::string> writtenCompressed;
    writtenCompressed.reserve(33554432 / 2);

    // Correct output ordering
    static const int order[25] = {
        12, 7, 8, 13, 18,
        17,16,11,6,2,
        3,4,9,14,19,
        24,23,22,21,20,
        15,10,5,0,1
    };

    std::vector<int> allRules(33554432);
    std::iota(allRules.begin(), allRules.end(), 0);

    std::mutex mtx;

    std::for_each(std::execution::par, allRules.begin(), allRules.end(),
        [&](int i)
        {
            // Canonical representative only
            if (FindLowestNeighborhoodValue(i) != i) return;

            if (!saveRule.R2MAP[i]) return;

            Neighborhood n = ConvertIntToNeighborhood(i);

            std::string compressed(25, '0');

            // Build in FILE ORDER
            for (int outIndex = 0; outIndex < 25; ++outIndex)
            {
                int srcBit = order[outIndex];
                compressed[outIndex] = n[srcBit] ? '1' : '0';
            }

            // Apply neighbor-x compression (still using raw bit identity)
            for (int outIndex = 0; outIndex < 25; ++outIndex)
            {
                int srcBit = order[outIndex];
                int neighborRule = i ^ (1 << srcBit);

                if (neighborRule >= 0 &&
                    neighborRule < 33554432 &&
                    saveRule.R2MAP[neighborRule])
                {
                    compressed[outIndex] = 'x';
                }
            }

            // Thread-safe insert + write
            std::lock_guard<std::mutex> lock(mtx);
            if (writtenCompressed.insert(compressed).second)
            {
                outFile << compressed << "\n";
            }
        });

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