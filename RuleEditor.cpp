// RuleEditor.cpp
#include "RuleEditor.h"
#include "R2INT_File.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <random>
#include <iostream>

RuleEditor::RuleEditor(std::mt19937& gen, const sf::Font& font)
    : clearText(font, "Clear Rule", 48),
    saveText(font, "Save Rule", 48),
    settingsSprite(settingsTexture)
{
    static std::uniform_int_distribution<int> rnd(0, 7);
    for (int i = 0; i < 25; i++)
        editorNeighborhood[i] = rnd(gen) > 4 ? 1 : 0;

    clearText.setPosition({ 730, 10 });
    clearText.setFillColor(sf::Color::Black);

    saveText.setPosition({ 730, 670 });
    saveText.setFillColor(sf::Color::Black);

    // Load settings icon from file
    if (!settingsTexture.loadFromFile("Settings.png")) {
        std::cerr << "Failed to load settings icon from file" << std::endl;
    }
    settingsSprite.setTexture(settingsTexture);

    sf::Rect<int> textureRect({ 0, 0 }, { 135, 135 });
    settingsSprite.setTextureRect(textureRect);
    settingsSprite.setPosition({ 1295.f, 10.f });
    settingsBounds = settingsSprite.getGlobalBounds();
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

        // Now you can test buttons
        if (clearText.getGlobalBounds().contains(mouseCoords)) {
            globalRule.ClearRule();
        }
        else if (saveText.getGlobalBounds().contains(mouseCoords)) {
            SaveTor2intFile(globalRule);
        }
        else if (mouseCoords.x >= 720.f) {  // clicking on the “result cell” area
            globalRule.ToggleIsotropicTransition(editorNeighborhood);
        }
        else {
            // grid editing logic here
            int modifyID = static_cast<int>(mouseCoords.x / 144) + 5 * static_cast<int>(mouseCoords.y / 144);
            editorNeighborhood[modifyID] = 1 - editorNeighborhood[modifyID];
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

    window->clear(sf::Color(0, 160, 80, 240));
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

    window->draw(clearText);
    window->draw(saveText);
    window->draw(settingsSprite);
    window->display();
}