#include "World.h"
#include <iostream>

#define DEBUG_BG

World::World() {
    Chunk initial;
    initial.CoordinateX = 0;
    initial.CoordinateY = 0;
    contents[{0, 0}] = initial;
}

void World::PaintAtCell(sf::Vector2i p, int newState)
{
    // Determine which grid the cell belongs to
    int gx = (p.x >= 0) ? p.x / GRID_DIMENSIONS : (p.x - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
    int gy = (p.y >= 0) ? p.y / GRID_DIMENSIONS : (p.y - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;

    // Local cell coordinates (wrapped into valid range)
    int lx = ((p.x % GRID_DIMENSIONS) + GRID_DIMENSIONS) % GRID_DIMENSIONS;
    int ly = ((p.y % GRID_DIMENSIONS) + GRID_DIMENSIONS) % GRID_DIMENSIONS;

    GridCoord coord = { gx, gy };

    // Get or create the grid
    auto& grid = contents[coord];
    grid.CoordinateX = gx;
    grid.CoordinateY = gy;

    int oldState = grid.Grid[lx][ly];

    // Paint the cell
    grid.Grid[lx][ly] = newState;
    grid.OldGrid[lx][ly] = newState;

    // Update Fill count
    grid.Fill += (newState - oldState);

    // Remove the chunk if it is empty after erasing
    if (newState == 0 && grid.Fill == 0) {
        contents.erase(coord);
    }
}

__int8 World::GetCellStateAt(sf::Vector2i p)
{
    // Determine which grid the cell is in
    int gx = (p.x >= 0) ? p.x / GRID_DIMENSIONS : (p.x - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
    int gy = (p.y >= 0) ? p.y / GRID_DIMENSIONS : (p.y - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;

    // Local cell coordinates within the grid
    int lx = p.x - gx * GRID_DIMENSIONS;
    int ly = p.y - gy * GRID_DIMENSIONS;

    GridCoord coord = { gx, gy };

    auto it = contents.find(coord);
    if (it != contents.end())
        return it->second.Grid[lx][ly];

    // Default background state (flickers with B0)
    return VoidState;
}

__int8 World::GetCellStateAtOld(sf::Vector2i p)
{
    // Determine which grid the cell is in
    int gx = (p.x >= 0) ? p.x / GRID_DIMENSIONS : (p.x - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;
    int gy = (p.y >= 0) ? p.y / GRID_DIMENSIONS : (p.y - GRID_DIMENSIONS + 1) / GRID_DIMENSIONS;

    // Local cell coordinates within the grid
    int lx = p.x - gx * GRID_DIMENSIONS;
    int ly = p.y - gy * GRID_DIMENSIONS;

    GridCoord coord = { gx, gy };

    auto it = contents.find(coord);
    if (it != contents.end())
        return it->second.OldGrid[lx][ly];

    // Default background state
    return VoidState;
}

void World::LinkAllNeighbors()
{
    for (auto& [coord, grid] : contents)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                GridCoord neighborCoord = { coord.x + dx, coord.y + dy };

                auto it = contents.find(neighborCoord);
                if (it != contents.end())
                {
                    grid.neighborGrids[dx + 1][dy + 1] = &it->second;
                }
                else
                {
                    grid.neighborGrids[dx + 1][dy + 1] = nullptr;
                }
            }
        }
    }
}

void World::Simulate(const R2INTRules& Rules) {
    // Step 1: Ensure needed neighbors exist
    std::vector<GridCoord> keys;
    keys.reserve(contents.size());
    for (const auto& [coord, grid] : contents)
        keys.push_back(coord);

    for (const GridCoord& coord : keys) {
        Chunk& grid = contents.at(coord);
        EnsureNeighborsExist(*this, grid);
    }

    // Step 3: Relink neighbors after changes
    LinkAllNeighbors();

    // Step 4: Simulate all grids safely
    keys.clear();
    for (const auto& [coord, grid] : contents)
        keys.push_back(coord);

    for (const GridCoord& coord : keys) {
        Chunk& grid = contents.at(coord);
        grid.Simulate(Rules, *this);
    }

    // Simulate the VoidState under the rules for B0 handling
    VoidState = ApplyRules(VoidState == 1 ? 33554431 : 0, Rules) ? 1 : 0;

    for (const GridCoord& coord : keys) {
        Chunk& grid = contents.at(coord);
        grid.ResetOld();
    }


    // Save DeleteEmptyGrids for last to prevent issues during simulation
    DeleteEmptyGrids(contents, VoidState);

    //std::cout << "[DEBUG] Number of grids after simulation: " << contents.size() << std::endl;
}

Chunk* World::GetNeighborGrid(int x, int y) {
    GridCoord coord = { x, y };

    // Check if the neighbor grid exists in the map
    auto it = contents.find(coord);
    if (it != contents.end()) {
        return &it->second;  // Return the existing grid
    }

    // If it doesn't exist, create and link the new grid
    Chunk newGrid(coord.x, coord.y);
    contents[coord] = newGrid;

    return &contents[coord];
}

void World::EnsureAllPotentialNeighborGridsExist() {
    std::vector<GridCoord> currentKeys;
    for (const auto& [coord, _] : contents)
        currentKeys.push_back(coord);

    for (const auto& coord : currentKeys) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                GridCoord neighborCoord = { coord.x + dx, coord.y + dy };
                if (contents.find(neighborCoord) == contents.end()) {
                    Chunk& newGrid = contents[neighborCoord];
                    newGrid.CoordinateX = neighborCoord.x;
                    newGrid.CoordinateY = neighborCoord.y;
                }
            }
        }
    }
}


void DeleteEmptyGrids(std::unordered_map<GridCoord, Chunk>& worldMap, __int8 VoidState) {
    for (auto it = worldMap.begin(); it != worldMap.end(); ) {
        if ((it->second.Fill == 0 && VoidState == 0) || (it->second.Fill == 4096 && VoidState == 1)) {
            it = worldMap.erase(it);
        }
        else {
            ++it;
        }
    }
}

// Draw
void World::Draw(sf::RenderWindow& window, float cellSize, const std::vector<sf::Color>& colors)
{
    size_t totalGrids = contents.size();
    sf::VertexArray vertexArray(sf::PrimitiveType::Triangles, totalGrids * GRID_DIMENSIONS * GRID_DIMENSIONS * 6);

    size_t vertexIndex = 0;

    for (const auto& entry : contents) {
        const GridCoord& gridCoord = entry.first;
        const Chunk& gridData = entry.second;

        float offsetX = gridCoord.x * GRID_DIMENSIONS * cellSize;
        float offsetY = gridCoord.y * GRID_DIMENSIONS * cellSize;

#ifdef DEBUG_BG
        // Deterministic debug background color
        std::size_t hashValue = std::hash<int>()(gridCoord.x) ^ (std::hash<int>()(gridCoord.y) << 1);
        unsigned char bgR = static_cast<unsigned char>((hashValue & 0xFF) % 32);
        unsigned char bgG = static_cast<unsigned char>(((hashValue >> 8) & 0xFF) % 32);
        unsigned char bgB = static_cast<unsigned char>(((hashValue >> 16) & 0xFF) % 32);
        sf::Color gridBgColor(bgR, bgG, bgB);
#else
        sf::Color gridBgColor(0, 0, 0); // Release build fixed background
#endif

        for (int i = 0; i < GRID_DIMENSIONS; i++) {
            for (int j = 0; j < GRID_DIMENSIONS; j++) {
                float x = offsetX + i * cellSize;
                float y = offsetY + j * cellSize;

                __int8 cellState = GetCellStateAt({
                    gridCoord.x * GRID_DIMENSIONS + i,
                    gridCoord.y * GRID_DIMENSIONS + j
                    });

                sf::Color color = (cellState == 0) ? gridBgColor : colors[cellState];

                // First triangle
                vertexArray[vertexIndex++].position = sf::Vector2f(x, y);
                vertexArray[vertexIndex++].position = sf::Vector2f(x, y + cellSize);
                vertexArray[vertexIndex++].position = sf::Vector2f(x + cellSize, y);

                // Second triangle
                vertexArray[vertexIndex++].position = sf::Vector2f(x + cellSize, y);
                vertexArray[vertexIndex++].position = sf::Vector2f(x, y + cellSize);
                vertexArray[vertexIndex++].position = sf::Vector2f(x + cellSize, y + cellSize);

                for (int k = 0; k < 6; k++)
                    vertexArray[vertexIndex - 6 + k].color = color;
            }
        }
    }

    window.draw(vertexArray);
}

sf::IntRect World::GetRect() const
{
    bool foundAny = false;

    int minLeft = 0;
    int minTop = 0;
    int maxRight = 0;
    int maxBottom = 0;

    for (const auto& [coord, chunk] : contents)
    {
        sf::IntRect r = chunk.GetRect();
        if (r.size.x < 0 || r.size.y < 0)
            continue; // empty chunk
        int globalLeft = chunk.CoordinateX * GRID_DIMENSIONS + r.position.x;
        int globalTop = chunk.CoordinateY * GRID_DIMENSIONS + r.position.y;

        int globalRight = globalLeft + r.size.x;
        int globalBottom = globalTop + r.size.y;

        if (!foundAny)
        {
            minLeft = globalLeft;
            minTop = globalTop;
            maxRight = globalRight;
            maxBottom = globalBottom;
            foundAny = true;
        }
        else
        {
            minLeft = std::min(minLeft, globalLeft);
            minTop = std::min(minTop, globalTop);
            maxRight = std::max(maxRight, globalRight);
            maxBottom = std::max(maxBottom, globalBottom);
        }
    }

    if (!foundAny)
        return sf::IntRect({ -1, -1 }, { -1, -1 });

    return sf::IntRect(
        { minLeft, minTop },
        { maxRight - minLeft, maxBottom - minTop }
    );
}

void World::PrintRLE() const
{
    sf::IntRect rect = GetRect();
    std::cout << "x = " << rect.size.x << ", y = " << rect.size.y << std::endl;
}