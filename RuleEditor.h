// RuleEditor.h
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include "OffsetStruct.h"  // For Neighborhood type and methods

class RuleEditor {
public:
    RuleEditor();

    void Draw(sf::RenderWindow* window,
        const Neighborhood editorNeighborhood,
        const std::vector<sf::Color>& colors,
        const std::vector<sf::Color>& ruleEditorColors,
        const R2INTRules& globalRule,
        const sf::Text& clearText,
        const sf::Text& saveText);
};