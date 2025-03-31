#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <random>
#include <iostream>
#include <filesystem>

#include "OffsetStruct.h"
#include "Grid.h"

R2INTRules globalRule;

void InitializeRule()
{
    std::cout << "Loading. . ." << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd(0, 6);

    for (unsigned int i = 0; i < 33554432; i++)
    {
        Neighborhood n = ConvertIntToNeighborhood(i);
        globalRule[i] = 0;
        // Jellyfish (R2,C2,S4-8,17-20,B7-8,10,16) from R2INT's Rule Collection
        int8_t neighbors = n.countOuterCells();
        if (n[12] == 0)
        {
            // Dead
            if (neighbors == 7 || neighbors == 8 || neighbors==10 || neighbors == 16)
            {
                globalRule[i] = 1;
            }
        }
        else
        {
            // Alive
            if ((neighbors >= 4 && neighbors <= 8) || (neighbors >= 17 && neighbors <= 20))
            {
                globalRule[i] = 1;
            }
        }

        if (i % 2097152 == 2097151)
        {
            std::cout << (i * 100 + 100) / 33554432 << "% complete." << std::endl;
        }
    }
}

int main() {
    std::cout << "Initializing rule..." << std::endl;
    InitializeRule();
    std::cout << "Initializing rule complete." << std::endl;
    std::cout << "Initializing grid..." << std::endl;
    Grid64 currentGrid;
    Grid64 originalGrid = currentGrid;
    std::cout << "Initialize grid complete!" << std::endl;

    float timeStep = 1.0f / 60.0f;  // 60 updates per second
    float accumulator = 0.0f;

    sf::Clock clock;
    int frameCount = 0;
    float elapsedTime = 0.0f;

    sf::Color colors[11];
    colors[0] = sf::Color::Black;
    colors[1] = sf::Color::White;
    colors[2] = sf::Color::Cyan;
    colors[3] = sf::Color::Magenta;
    colors[4] = sf::Color::Yellow;
    colors[5] = sf::Color::Blue;
    colors[6] = sf::Color::Green;
    colors[7] = sf::Color::Red;
    colors[8] = sf::Color(127, 255, 255, 255);
    colors[9] = sf::Color(255, 127, 255, 255);
    colors[10] = sf::Color(255, 255, 127, 255);

    sf::RenderWindow window(sf::VideoMode({ 1600, 1600 }), "R2INT");
    std::unique_ptr<sf::RenderWindow> secondWindow = nullptr;

    // Load font
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return -1;
    }

    sf::Text menuText(font, "Open New Window", 24);
    menuText.setPosition({ 10, 10 });
    menuText.setFillColor(sf::Color::White);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd(0, 7);

    sf::View view;
    view.setSize(static_cast<sf::Vector2f>(window.getSize()));
    view.setCenter({ view.getSize().x / 2 , view.getSize().y / 2});

    while (window.isOpen()) {  // Replace `mainWindow` with `window`
        while (const std::optional event = window.pollEvent()) {  // Use `window` for event polling
            if (event->is<sf::Event::Closed>()) {
                window.close();
                if (secondWindow) {
                    secondWindow->close();
                    secondWindow.reset();  // Close and delete second window safely
                }
            }
            else if (event->is<sf::Event::MouseButtonPressed>()) {
                

                // Check if the mouse position is within the bounds of the menu text
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                if (menuText.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePosition))) {
                    if (!secondWindow) {
                        secondWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode({ 1280, 720 }), "Second Window");
                    }
                }
            }
            else if (event->is<sf::Event::KeyPressed>()) {
                if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::R)
                {
                    std::cout << "Reset!" << std::endl;
                    currentGrid = originalGrid;
                }
            }
            else if (event->is<sf::Event::Resized>()) {
                view.setSize(static_cast<sf::Vector2f>(window.getSize()));
                view.setCenter({ view.getSize().x / 2 , view.getSize().y / 2 });
                window.setView(view);
            }
        }

        float deltaTime = clock.restart().asSeconds();
        accumulator += deltaTime;
        elapsedTime += deltaTime;

        // Process the grid update at a fixed timestep
        while (accumulator >= timeStep) {
            currentGrid.Simulate(globalRule);
            accumulator -= timeStep;
        }

        deltaTime = clock.restart().asSeconds();
        elapsedTime += deltaTime;
        frameCount++;

        if (elapsedTime >= 1.f)
        {
            std::cout << "FPS: " << frameCount << std::endl;
            elapsedTime = 0.0f;
            frameCount = 0;
        }

        window.clear(sf::Color::Black);

        sf::VertexArray grid(sf::PrimitiveType::Triangles, GRID_DIMENSIONS * GRID_DIMENSIONS * 6);

        int cellSize = 8;

        for (int i = 0; i < GRID_DIMENSIONS; i++) {
            for (int j = 0; j < GRID_DIMENSIONS; j++) {
                int index = (i * GRID_DIMENSIONS + j) * 6;
                float x = i * cellSize;
                float y = j * cellSize + 50;

                sf::Color color = colors[currentGrid.Grid[i][j]];

                // First Triangle (Bottom-left, Top-left, Top-right)
                grid[index].position = { x, y };
                grid[index + 1].position = { x, y + cellSize };
                grid[index + 2].position = { x + cellSize, y };

                // Second Triangle (Top-right, Bottom-right, Bottom-left)
                grid[index + 3].position = { x + cellSize, y };
                grid[index + 4].position = { x, y + cellSize };
                grid[index + 5].position = { x + cellSize, y + cellSize };

                // Apply color to all triangle vertices
                for (int k = 0; k < 6; k++) {
                    grid[index + k].color = color;
                }
            }
        }

        window.draw(grid);
        window.draw(menuText);
        window.display();

        if (secondWindow && secondWindow->isOpen()) {
            while (const std::optional secondEvent = secondWindow->pollEvent()) {
                if (secondEvent->is<sf::Event::Closed>()) {
                    secondWindow->close();
                    secondWindow.reset();  // Properly close and delete the second window
                    break;
                }
            }

            if (secondWindow)
            {
                secondWindow->clear(sf::Color::Blue);
                secondWindow->display();
            }
        }
    }

    return 0;
}
