// RuleEditor.cpp
#include "RuleEditor.h"
#include "R2INT_File.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "gui.h"
#include <random>
#include <iostream>

// Grid parameters
const int rows = 5;
const int cols = 2;
const float boxWidth = 544.f;
const float boxHeight = 96.f;
const float startX = 72.f;   // top-left x
const float startY = 72.f;   // top-left y
const float spacingX = 72.f; // horizontal spacing
const float spacingY = 24.f; // vertical spacing

RuleEditor::RuleEditor(std::mt19937& gen, const sf::Font& font, R2INTRules& globalRule)
    : clearText(font, "Clear Rule", 48),
    saveText(font, "Save Rule", 48),
    settingsSprite(settingsTexture),
    settingsMenu(5,2,
        { 544.f, 96.f },
        { 72.f, 72.f },
        { 72.f, 24.f },
        font,
        { "Clear", "Save", "Load", "Set Rule", "Rand Rule", "Mutate", "Set Nbrhood", "Set States", "Set Dimension", "Set Symmetry" }, 80
    )
{
    static std::uniform_int_distribution<int> rnd(0, 7);
    for (int i = 0; i < 25; i++)
        editorNeighborhood[i] = rnd(gen) > 4 ? 1 : 0;

    f = font;

    clearText.setPosition({ 730, 10 });
    clearText.setFillColor(sf::Color::Black);

    saveText.setPosition({ 730, 670 });
    saveText.setFillColor(sf::Color::Black);

    // Load settings icon from file
    if (!settingsTexture.loadFromFile("assets\\Settings.png")) {
        std::cerr << "Failed to load settings icon from file" << std::endl;
    }
    settingsSprite.setTexture(settingsTexture);

    sf::Rect<int> textureRect({ 0, 0 }, { 135, 135 });
    settingsSprite.setTextureRect(textureRect);
    settingsSprite.setPosition({ 1295.f, 10.f });
    settingsBounds = settingsSprite.getGlobalBounds();

    settingsMenu.SetButtonCallback(0, [this, &globalRule]() {
        globalRule.ClearRule();
        screen = 0;
        });
    settingsMenu.SetButtonCallback(1, [this, &globalRule]() {
        SaveTor2intFile(globalRule);
        screen = 0;
        });
    settingsMenu.SetButtonCallback(2, [this, &globalRule]() {
        LoadFromr2intFile(globalRule);
        screen = 0;
        });
}

void RuleEditor::RandomizeNeighborhood(std::mt19937& gen) {
    static std::uniform_int_distribution<int> rnd(0, 7);

    for (int i = 0; i < 25; i++)
        editorNeighborhood[i] = rnd(gen) > 4 ? 1 : 0;
}

void RuleEditor::HandleEvent(const sf::Event& event,
    R2INTRules& globalRule,
    std::mt19937& gen,
    sf::RenderWindow& window)
{
    const bool INVERT_CONTROLS = true;
    int dx = 0, dy = 0;

    if (event.is<sf::Event::MouseButtonPressed>()) {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(window);  // window-relative
        sf::Vector2f mouseCoords = window.mapPixelToCoords(pixelPos);  // convert to world/UI coords

        if (settingsBounds.contains(mouseCoords)) {
            if (screen == 0) screen = 1;
            else screen = 0;
        }
        else if (screen == 1) {
            settingsMenu.handleClick(mouseCoords);
        }
        else if (screen == 0)
        {
            if (mouseCoords.x >= 720.f) {  // clicking on the “result cell” area
                globalRule.ToggleIsotropicTransition(editorNeighborhood);
            }
            else {
                // grid editing logic here
                int modifyID = static_cast<int>(mouseCoords.x / 144) + 5 * static_cast<int>(mouseCoords.y / 144);
                editorNeighborhood[modifyID] = 1 - editorNeighborhood[modifyID];
            }
        }
    }
    if (event.is<sf::Event::KeyPressed>()) {
        auto key = event.getIf<sf::Event::KeyPressed>()->code;

        switch (key) {
        case sf::Keyboard::Key::R:
            RandomizeNeighborhood(gen);
            break;
        case sf::Keyboard::Key::Left:
            dx = INVERT_CONTROLS ? 1 : -1;
            break;
        case sf::Keyboard::Key::Right:
            dx = INVERT_CONTROLS ? -1 : 1;
            break;
        case sf::Keyboard::Key::Up:
            dy = INVERT_CONTROLS ? 1 : -1;
            break;
        case sf::Keyboard::Key::Down:
            dy = INVERT_CONTROLS ? -1 : 1;
            break;
        default:
            break;
        }

        if (dx != 0 || dy != 0)
            editorNeighborhood = ShiftNeighborhood(editorNeighborhood, dx, dy);
    }
}


void RuleEditor::Draw(sf::RenderWindow* window,
    const std::vector<sf::Color>& colors,
    const std::vector<sf::Color>& ruleEditorColors,
    const R2INTRules& globalRule)
{
    if (!window) return;
    
    if (screen == 0) {
        window->clear(sf::Color(0, 160, 80, 255));
        sf::RectangleShape rc;

        // Draw the 5x5 grid of cells
        for (int j = 0; j < 5; j++) {
            for (int i = 0; i < 5; i++) {
                int index = 5 * j + i;

                if (i == 2 && j == 2) // Center cell
                    rc.setFillColor(colors[editorNeighborhood[index]]);
                else // Other cells
                    rc.setFillColor(ruleEditorColors[editorNeighborhood[index]]);

                rc.setPosition({ i * 144.f + 8.f, j * 144.f + 8.f });
                rc.setSize({ 128.f, 128.f });
                window->draw(rc);
            }
        }

        int TransitionID = ConvertNeighborhoodToInt(editorNeighborhood);
        rc.setFillColor(ruleEditorColors[globalRule[TransitionID]]);

        // Draw the resulting cell state
        rc.setPosition({ 984.f, 260.f });
        rc.setSize({ 192.f, 192.f });
        window->draw(rc);
    }
    else if (screen == 1) {
        window->clear(sf::Color(0, 120, 60, 255));

        sf::Vector2f mousePos =
            static_cast<sf::Vector2f>(sf::Mouse::getPosition(*window));
        settingsMenu.draw(
            *window,
            mousePos,
            [](int index, bool hovered) -> sf::Color {
                if (index <= 2) {
                    return hovered
                        ? sf::Color(40, 200, 120)
                        : sf::Color(100, 255, 170);
                }
                else {
                    return sf::Color(0, 30, 15);
                }
            }
        );
    }

    window->draw(settingsSprite);
    window->display();
}