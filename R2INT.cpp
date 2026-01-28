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
#include "gui.h"

R2INTRules globalRule;

void InitializeRule()
{
    std::cout << "Initializing rule..." << std::endl;

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

    std::cout << "Initializing rule complete." << std::endl;
}

int main() {
    InitializeRule();
    std::cout << "Initializing grid..." << std::endl;
    World currentWorld;
    World originalWorld = currentWorld;
    std::cout << "Initialize grid complete!" << std::endl;

    // Load font
    sf::Font font;
    if (!font.openFromFile("assets\\arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return -1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd(0, 7);

    RuleEditor ruleEditor(gen, font, globalRule);

    //
    // Playback variables
    //
    bool isPlaying = false;

    // Timer variables
    float timeStep = 1.0f / 60.0f;  // 60 updates per second
    float accumulator = 0.0f;
    sf::Clock clock;
    int frameCount = 0;
    float elapsedTime = 0.0f;

    // Edit variables
    int drawingState = 0;

    // Colors for multistate rules (obtined by an algorithm I made; is it possible to make that run faster than O(n)?)
    std::vector<sf::Color> colors(4);
    colors[0] = sf::Color::Black;
    colors[1] = sf::Color::White;
    colors[2] = sf::Color::Cyan;
    colors[3] = sf::Color::Magenta;

    // RuleEditor colors is obtained by performing simple mat to the standard colors
    // R: 0.875r, G = 0.875g, B = 0.6875b + 32
    std::vector<sf::Color> ruleEditorColors(4);
    ruleEditorColors[0] = sf::Color::Color(0, 64, 32);
    ruleEditorColors[1] = sf::Color::Color(224, 255, 240);
    ruleEditorColors[2] = sf::Color::Color(0, 255, 240);
    ruleEditorColors[3] = sf::Color::Color(224, 64, 240);

    sf::RenderWindow window(sf::VideoMode({ 1024, 768 }), "R2INT");
    std::unique_ptr<sf::RenderWindow> secondWindow = nullptr;
    window.setFramerateLimit(60);

    // Camera variables
    sf::Vector2f previousMousePosition;
    sf::Vector2i worldPrevious;

    bool isRightMouseDown = false;
    bool isLeftMouseDown = false;

    sf::View view;
    view.setSize(static_cast<sf::Vector2f>(window.getSize()));
    view.setCenter({ view.getSize().x / 2 , view.getSize().y / 2});

    sf::Vector2u newSize = window.getSize();
    sf::Vector2f newSizef(static_cast<sf::Vector2f>(newSize));

    // GUI setup
    sf::View uiView;
    uiView.setSize(newSizef);
    uiView.setCenter(newSizef * 0.5f);

    MainGUI mainGui(window.getSize());
    MenuManager menuManager;
    menuManager.SetColorFunction([](int index, bool hovered) {
        return hovered ? sf::Color(128, 255, 192) : sf::Color(128, 160, 144);
        });
    Menu settingsMenu(1, 2, { 384.f, 72.f }, { 72.f, 72.f }, { 60.f, 24.f }, font, { "Pattern", "Rule Editor" }, 64 );
    settingsMenu.centerIn(newSize);
    Menu patternMenu(1, 2, { 384.f, 72.f }, { 72.f, 72.f }, { 60.f, 24.f }, font,
        { "Clear", "Randomize" }, 64);
    patternMenu.centerIn(newSize);

    // Make the callbacks for the main GUI buttons
    auto PlayPause = [&]() {
        isPlaying = !isPlaying;
        sf::Color playColor = isPlaying ? sf::Color(0, 192, 96) : sf::Color(0, 255, 128);
        sf::Texture& texture = isPlaying ? mainGui.pauseTex : mainGui.playTex;
        mainGui.playButton.setColor(playColor);
        mainGui.playButton.SetIcon(texture);
        };
    auto Reset = [&]() {
        currentWorld = originalWorld;
        isPlaying = false;
        mainGui.playButton.setColor(sf::Color(0, 255, 128));
        mainGui.playButton.SetIcon(mainGui.playTex);
        };
    auto ToggleSettingsMenu = [&]() {
        menuManager.Toggle("Settings");
        };
    auto OpenRuleEditor = [&]() {
        if (!secondWindow) {
            secondWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode({ 1440, 720 }), "R2INT - Rule Editor");
        }
        isLeftMouseDown = false;
        isRightMouseDown = false;
        menuManager.Close();
        };
    auto OpenPatternsMenu = [&]() {
        menuManager.Open("Patterns");
        };
    auto ClearPattern = [&]() {
        currentWorld = World();
        originalWorld = currentWorld;
        isPlaying = false;
        mainGui.playButton.setColor(sf::Color(0, 255, 128));
        mainGui.playButton.SetIcon(mainGui.playTex);
        menuManager.Close();
        };
    auto Randomize = [&]() {
        currentWorld.TestRandomize();
        originalWorld = currentWorld;
        isPlaying = false;
        mainGui.playButton.setColor(sf::Color(0, 255, 128));
        mainGui.playButton.SetIcon(mainGui.playTex);
        menuManager.Close();
        };
    mainGui.playButton.SetCallback(PlayPause);
    mainGui.resetButton.SetCallback(Reset);
    mainGui.settingsButton.SetCallback(ToggleSettingsMenu);
    settingsMenu.SetButtonCallback(0, OpenPatternsMenu);
    settingsMenu.SetButtonCallback(1, OpenRuleEditor);
    patternMenu.SetButtonCallback(0, ClearPattern);
    patternMenu.SetButtonCallback(1, Randomize);

    // Finally, add the menus to the manager
    menuManager.AddMenu("Settings", std::move(settingsMenu));
    menuManager.AddMenu("Patterns", std::move(patternMenu));

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
                // Handle CA interaction
                window.setView(view);
                if (event->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Right)
                {
                    isRightMouseDown = true;

                    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(pixelPos, view);

                    sf::Vector2i pos = currentWorld.GetWorldCoords(mouseWorldPos);
                    drawingState = (currentWorld.GetCellStateAt(pos) + 1) % currentWorld.n_states;
                }
                else if (event->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left)
                {
                    isLeftMouseDown = true;
                    previousMousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                }
                // Handle GUI clicks
                window.setView(uiView);

                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

                mainGui.HandleMouseClick(static_cast<sf::Vector2f>(mousePosition));
                menuManager.HandleClick(static_cast<sf::Vector2f>(mousePosition));
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
                    Reset();
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
                    PlayPause();
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

                mainGui.Resize(newSize);
                menuManager.centerMenus(newSize);
            }
        }

        float deltaTime = clock.restart().asSeconds();
        accumulator += deltaTime * (isPlaying ? 1 : 0);
        elapsedTime += deltaTime;

        // Process the grid update at a fixed timestep
        while (accumulator >= timeStep) {
            currentWorld.Simulate(globalRule);
            //currentWorld.PrintRLE();
            
            accumulator -= timeStep;
        }

        deltaTime = clock.restart().asSeconds();
        elapsedTime += deltaTime;
        frameCount++;

        if (elapsedTime >= 32.f)
        {
            std::cout << "FPS: " << frameCount / 32.f << std::endl;
            elapsedTime = 0.0f;
            frameCount = 0;
        }

        window.clear(colors[currentWorld.VoidState]);
        window.setView(view);

        if (isRightMouseDown) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(pixelPos, view);

            sf::Vector2i pos = currentWorld.GetWorldCoords(mouseWorldPos);

            if (currentWorld.Generation == 0)
                originalWorld.PaintAtCell(pos, drawingState);

            currentWorld.PaintAtCell(pos, drawingState);
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
        currentWorld.Draw(window, colorVec);

        // Draw UI
        window.setView(uiView);
        menuManager.Draw(window, static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)));
        mainGui.Draw(window);

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
