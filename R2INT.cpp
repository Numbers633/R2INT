#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <array>
#include <filesystem>
#include <iostream>
#include <functional> // for std::hash
#include <random>
#include <string>

#include "World.h"
#include "OffsetStruct.h"
#include "R2INT_File.h"
#include "RuleEditor.h"

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
    World currentWorld;
    World originalWorld = currentWorld;
    std::cout << "Initialize grid complete!" << std::endl;

    // Load font
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return -1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd(0, 7);

    RuleEditor ruleEditor(gen, font);

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

    float cellSize = 10.f;
    int n_states = 2;

    // Edit variables
    int drawingState = 0;

    // Colors for multistate rules
    std::vector<sf::Color> colors(11);
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

    std::vector<sf::Color> ruleEditorColors(4);
    ruleEditorColors[0] = sf::Color::Color(0, 64, 32);
    ruleEditorColors[1] = sf::Color::Color(224, 255, 240);
    ruleEditorColors[2] = sf::Color::Color(0, 255, 240);
    ruleEditorColors[3] = sf::Color::Color(224, 64, 240);

    sf::RenderWindow window(sf::VideoMode({ 640, 640 }), "R2INT");
    std::unique_ptr<sf::RenderWindow> secondWindow = nullptr;
    window.setFramerateLimit(60);

    // Camera variables
    sf::Vector2f previousMousePosition;
    sf::Vector2i worldPrevious;

    bool isRightMouseDown = false;
    bool isLeftMouseDown = false;

    sf::Text menuText(font, "Open Rule Editor", 24);
    menuText.setPosition({ 10, 10 });
    menuText.setFillColor(sf::Color::White);

    sf::View view;
    view.setSize(static_cast<sf::Vector2f>(window.getSize()));
    view.setCenter({ view.getSize().x / 2 , view.getSize().y / 2});

    sf::View uiView;

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
                    float y = mouseWorldPos.y;

                    // Global cell coordinates
                    int i = static_cast<int>(std::floor(x / cellSize));
                    int j = static_cast<int>(std::floor(y / cellSize));

                    drawingState = (currentWorld.GetCellStateAt({ i, j }) + 1) % n_states;
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
                    currentWorld = originalWorld;
                    generation = 0;
                    isPlaying = false;
                }
                else if (keyPress == sf::Keyboard::Key::Equal)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                        timeStep /= 1.18920711f; // Keep this
                    }
                    else {
                        view.zoom(0.5f); // Zoom IN (scale down view size)
                    }
                }
                else if (keyPress == sf::Keyboard::Key::Hyphen)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                        timeStep *= 1.18920711f; // Keep this
                    }
                    else {
                        view.zoom(2.0f); // Zoom OUT (scale up view size)
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
                // Update the view to match new window size, keeping the same center
                sf::Vector2u newSize = window.getSize();
                sf::Vector2f newSizef(static_cast<sf::Vector2f>(newSize));

                // Update world view
                view.setSize(newSizef); // Keeps zoom scale constant

                // Update UI view to match pixel coords (top-left = (0,0), bottom-right = (width, height))
                uiView.setSize(newSizef);
                uiView.setCenter(newSizef * 0.5f);
            }
        }

        float deltaTime = clock.restart().asSeconds();
        accumulator += deltaTime * (isPlaying ? 1 : 0);
        elapsedTime += deltaTime;

        // Process the grid update at a fixed timestep
        while (accumulator >= timeStep) {
            generation++;
            currentWorld.Simulate(globalRule);
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

        window.clear(colors[currentWorld.VoidState]);
        window.setView(view);

        if (isRightMouseDown) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(pixelPos, view);
            float x = mouseWorldPos.x;
            float y = mouseWorldPos.y;

            int i = (x >= 0.f) ? static_cast<int>(x / cellSize) : static_cast<int>((x - cellSize + 1.f) / cellSize);
            int j = (y >= 0.f) ? static_cast<int>(y / cellSize) : static_cast<int>((y - cellSize + 1.f) / cellSize);

            if (generation == 0)
                originalWorld.PaintAtCell({ i, j }, drawingState);

            currentWorld.PaintAtCell({ i, j }, drawingState);
        }
        if (isLeftMouseDown) {
            window.setView(view);  // Use current view for mapping

            sf::Vector2f currentMousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2f delta = previousMousePosition - currentMousePosition;
            view.move(delta);
            window.setView(view);
            previousMousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));  // <== Update after view.move
        }
        std::vector<sf::Color> colorVec(std::begin(colors), std::end(colors));
        currentWorld.Draw(window, cellSize, colorVec);

        // Draw UI
        window.setView(uiView);
        window.draw(menuText);

        window.display();

        if (secondWindow && secondWindow->isOpen()) {
            while (const std::optional secondEvent = secondWindow->pollEvent()) {
                if (secondEvent->is<sf::Event::Closed>()) {
                    secondWindow->close();
                    secondWindow.reset();  // Properly close and delete the second window
                    break;
                }

                ruleEditor.HandleEvent(*secondEvent, globalRule, gen, *secondWindow);
            }

            if (secondWindow)
            {
                ruleEditor.Draw(secondWindow.get(), colors, ruleEditorColors, globalRule);
            }
        }
    }

    return 0;
}
