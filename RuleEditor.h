// RuleEditor.h
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <array>
#include <random>
#include "OffsetStruct.h"  // For Neighborhood type and methods

class RuleEditor {
public:
    RuleEditor(std::mt19937& gen, const sf::Font& font);

    void RandomizeNeighborhood(std::mt19937& gen);
    void HandleEvent(const sf::Event& event,
        R2INTRules& globalRule,
        std::mt19937& gen,
        sf::RenderWindow& window);

    void Draw(sf::RenderWindow* window,
        const std::vector<sf::Color>& colors,
        const std::vector<sf::Color>& ruleEditorColors,
        const R2INTRules& globalRule);
private:
    Neighborhood editorNeighborhood{};
    sf::Text clearText;
    sf::Text saveText;
};