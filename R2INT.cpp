#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <random>
#include <iostream>
#include <filesystem>

#include "OffsetStruct.h"
#include "Grid.h"

R2INTRules globalRule;

#define PERCENT_INCREMENT 8388608
#ifdef _DEBUG
#define PERCENT_INCREMENT 2097152
#endif

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
        // Conway's Game of Life (B3/S23)
        int8_t neighbors = n[6] + n[7] + n[8] + n[11] + n[13] + n[16] + n[17] + n[18];
        if (n[12] == 0)
        {
            // Dead
            if (neighbors == 3)
            {
                globalRule[i] = 1;
            }
        }
        else
        {
            // Alive
            if (neighbors == 2 || neighbors == 3)
            {
                globalRule[i] = 1;
            }
        }

        if (i % PERCENT_INCREMENT == PERCENT_INCREMENT - 1)
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

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd(0, 7);

    //
    // Playback variables
    //
    bool isPlaying = false;
    
    float timeStep = 1.0f / 60.0f;  // 60 updates per second
    float accumulator = 0.0f;

    sf::Clock clock;
    int frameCount = 0;
    int generation = 0;
    float elapsedTime = 0.0f;

    float cellSize = 4.f;
    int n_states = 2;

    // Edit variables
    int drawingState = 0;
    
    // Variables for the rule editor
    Neighborhood  editorNeighborhood;
    for (int i = 0; i < 25; i++)
    {
        editorNeighborhood[i] = rnd(gen) > 4 ? 1 : 0;
    }

    // Colors for multistate rules
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

    sf::Color ruleEditorColors[4];
    ruleEditorColors[0] = sf::Color::Color(0, 64, 32);
    ruleEditorColors[1] = sf::Color::Color(224, 255, 240);
    ruleEditorColors[2] = sf::Color::Color(0, 255, 240);
    ruleEditorColors[3] = sf::Color::Color(224, 64, 240);

    sf::RenderWindow window(sf::VideoMode({ 800, 800 }), "R2INT");
    std::unique_ptr<sf::RenderWindow> secondWindow = nullptr;

    // Camera variables
    sf::Vector2f previousMousePosition;
    sf::Vector2i worldPrevious;

    bool isRightMouseDown = false;
    bool isLeftMouseDown = false;

    // Load font
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return -1;
    }

    sf::Text menuText(font, "Open Rule Editor", 24);
    menuText.setPosition({ 10, 10 });
    menuText.setFillColor(sf::Color::White);

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
                if (event->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Right)
                {
                    isRightMouseDown = true;

                    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(pixelPos, view);

                    float x = mouseWorldPos.x;
                    float y = mouseWorldPos.y - 50.f;

                    int i = static_cast<int>(x / cellSize);
                    int j = static_cast<int>(y / cellSize);

                    if (i >= 0 && i < GRID_DIMENSIONS && j >= 0 && j < GRID_DIMENSIONS) {
                        drawingState = (currentGrid.Grid[i][j] + 1) % n_states;
                    }
                }
                else if (event->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left)
                {
                    window.setView(view);
                    isLeftMouseDown = true;
                    previousMousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                }
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                if (menuText.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePosition))) {
                    if (!secondWindow) {
                        secondWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode({ 1440, 720 }), "R2INT - Rule Editor");
                    }
                    isLeftMouseDown = false;
                    isRightMouseDown = false;
                }
            }
            else if (event->is<sf::Event::MouseButtonReleased>()) {
                if (event->getIf<sf::Event::MouseButtonReleased>()->button == sf::Mouse::Button::Right)
                {
                    isRightMouseDown = false;
                }
                else if (event->getIf<sf::Event::MouseButtonReleased>()->button == sf::Mouse::Button::Left)
                {
                    isLeftMouseDown = false;
                }
            }
            else if (event->is<sf::Event::KeyPressed>()) {
                sf::Keyboard::Key keyPress = event->getIf<sf::Event::KeyPressed>()->code;
                if (keyPress == sf::Keyboard::Key::R)
                {
                    currentGrid = originalGrid;
                    generation = 0;
                    isPlaying = false;
                }
                else if (keyPress == sf::Keyboard::Key::Equal)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                        timeStep /= 1.18920711f; // 4th root of 2
                    }
                    else {
                        cellSize *= 2;
                    }
                }
                else if (keyPress == sf::Keyboard::Key::Hyphen)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                        timeStep *= 1.18920711f; // 4th root of 2
                    }
                    else {
                        cellSize /= 2;
                    }
                }
                else if (keyPress == sf::Keyboard::Key::Enter)
                {
                    isPlaying = !isPlaying;
                }
                else if (keyPress == sf::Keyboard::Key::Space)
                {
                    accumulator += timeStep;
                    isPlaying = false;
                }
            }
            else if (event->is<sf::Event::Resized>()) {
                view.setSize(static_cast<sf::Vector2f>(window.getSize()));
                view.setCenter({ view.getSize().x / 2 , view.getSize().y / 2 });
                window.setView(view);
            }
        }

        float deltaTime = clock.restart().asSeconds();
        accumulator += deltaTime * (isPlaying ? 1 : 0);
        elapsedTime += deltaTime;

        // Process the grid update at a fixed timestep
        while (accumulator >= timeStep) {
            generation++;
            currentGrid.Simulate(globalRule);
            accumulator -= timeStep;
        }

        deltaTime = clock.restart().asSeconds();
        elapsedTime += deltaTime;
        frameCount++;

        if (elapsedTime >= 10.f)
        {
            std::cout << "FPS: " << frameCount / 10.f << std::endl;
            elapsedTime = 0.0f;
            frameCount = 0;
        }

        window.clear(sf::Color::Black);

        if (isRightMouseDown) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(pixelPos, view);

            // Subtract vertical offset (50)
            float x = mouseWorldPos.x;
            float y = mouseWorldPos.y - 50.f;

            int i = static_cast<int>(x / cellSize);
            int j = static_cast<int>(y / cellSize);

            if (i >= 0 && i < GRID_DIMENSIONS && j >= 0 && j < GRID_DIMENSIONS) {
                if (generation == 0)
                {
                    originalGrid.Grid[i][j] = drawingState; // Set to alive
                    originalGrid.OldGrid[i][j] = drawingState;
                }
                currentGrid.Grid[i][j] = drawingState;
                currentGrid.OldGrid[i][j] = drawingState;
            }
        }
        if (isLeftMouseDown) {
            window.setView(view);  // Use current view for mapping

            sf::Vector2f currentMousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2f delta = previousMousePosition - currentMousePosition;
            view.move(delta);
            window.setView(view);
            previousMousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));  // <== Update after view.move
        }

        sf::VertexArray grid(sf::PrimitiveType::Triangles, GRID_DIMENSIONS * GRID_DIMENSIONS * 6);

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

        window.setView(view);
        window.draw(grid);

        // Draw the menu with the default (fixed) view
        window.setView(window.getDefaultView());
        window.draw(menuText);

        // Present the frame
        window.display();

        if (secondWindow && secondWindow->isOpen()) {
            sf::Text clearText(font, "Clear Rule", 24);
            clearText.setPosition({ 730, 10 });
            clearText.setFillColor(sf::Color::Black);
            while (const std::optional secondEvent = secondWindow->pollEvent()) {
                if (secondEvent->is<sf::Event::Closed>()) {
                    secondWindow->close();
                    secondWindow.reset();  // Properly close and delete the second window
                    break;
                }
                else if (secondEvent->is<sf::Event::MouseButtonPressed>()) {
                    sf::Vector2i pixelPos = sf::Mouse::getPosition(*secondWindow);
                    sf::Vector2f mouseWindowCoords = secondWindow->mapPixelToCoords(pixelPos, secondWindow->getDefaultView());

                    if (mouseWindowCoords.x >= 720.f) {
                        globalRule.ToggleIsotropicTransition(editorNeighborhood);
                        if (clearText.getGlobalBounds().contains(static_cast<sf::Vector2f>(pixelPos))) {
                            for (unsigned int i = 0; i < 33554432; i++)
                            {
                                Neighborhood n = ConvertIntToNeighborhood(i);
                                globalRule[i] = 0;
                                if (i % PERCENT_INCREMENT == PERCENT_INCREMENT - 1)
                                {
                                    std::cout << (i * 100 + 100) / 33554432 << "% complete." << std::endl;
                                }
                            }
                        }
                    }
                    else
                    {
                        int modifyID = floor(mouseWindowCoords.x / 144) + 5 * floor(mouseWindowCoords.y / 144);
                        editorNeighborhood[modifyID] = 1 - editorNeighborhood[modifyID];
                    }
                }
                else if (secondEvent->is<sf::Event::KeyPressed>()) {
                    sf::Keyboard::Key keyPress = secondEvent->getIf<sf::Event::KeyPressed>()->code;
                    if (keyPress == sf::Keyboard::Key::R)
                    {
                        for (int i = 0; i < 25; i++)
                        {
                            editorNeighborhood[i] = rnd(gen) > 4 ? 1 : 0;
                        }
                    }
                }
            }

            if (secondWindow)
            {
                secondWindow->clear(sf::Color::Color(0, 160, 80, 240));
                sf::RectangleShape rc;
                for (int i = 0; i < 5; i++)
                {
                    for (int j = 0; j < 5; j++)
                    {
                        if (i == 2 && j == 2)
                        {
                            rc.setFillColor(colors[editorNeighborhood[5 * j + i]]);
                        }
                        else
                        {
                            rc.setFillColor(ruleEditorColors[editorNeighborhood[5 * j + i]]);
                        }
                        
                        rc.setPosition({ i * 144.f + 8.f, j * 144.f + 8.f });
                        rc.setSize({ 128.f, 128.f });
                        secondWindow->draw(rc);
                    }
                }

                int TransitionID = ConvertNeighborhoodToInt(editorNeighborhood);
                rc.setFillColor(ruleEditorColors[globalRule[TransitionID]]);

                rc.setPosition({ 976.f + 8.f, 252.f + 8.f });
                rc.setSize({ 192.f, 192.f });
                secondWindow->draw(rc);
                secondWindow->draw(clearText);
                secondWindow->display();
            }
        }
    }

    return 0;
}
