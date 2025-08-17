#include <fstream>
#include <iostream>
#include <unordered_set>
#include <string>
#include <vector>
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

    std::unordered_set<std::string> writtenCompressed;
    writtenCompressed.reserve(33554432 / 2); // Rough estimate

    std::vector<int> allRules(33554432);
    std::iota(allRules.begin(), allRules.end(), 0);

    // Parallel loop
    std::mutex mtx;
    std::for_each(std::execution::par, allRules.begin(), allRules.end(), [&](int i) {
        if (FindLowestNeighborhoodValue(i) != i) return; // Skip non-minimal
        if (!saveRule.R2MAP[i]) return; // Skip inactive

        Neighborhood n = ConvertIntToNeighborhood(i);
        std::string compressed(25, '0');
        for (int bit = 0; bit < 25; bit++)
            compressed[bit] = n[bit] ? '1' : '0';

        // Neighbor-x compression
        for (int bit = 0; bit < 25; bit++) {
            int neighborRule = i ^ (1 << bit);
            if (neighborRule >= 0 && neighborRule < 33554432 && saveRule.R2MAP[neighborRule])
                compressed[bit] = 'x';
        }

        // Insert into the set (thread-safe with a mutex)
        std::lock_guard<std::mutex> lock(mtx);
        if (writtenCompressed.find(compressed) == writtenCompressed.end()) {
            writtenCompressed.insert(compressed);
            outFile << compressed << "\n";
        }
    });

    outFile.close();
    std::cout << "Save complete!" << std::endl;
}
