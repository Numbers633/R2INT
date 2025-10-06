#include "RuleEditor.h"
#include <SFML/Graphics.hpp>

RuleEditor::RuleEditor() = default;

void RuleEditor::Draw(sf::RenderWindow* window,
    const Neighborhood editorNeighborhood,
    const std::vector<sf::Color>& colors,
    const std::vector<sf::Color>& ruleEditorColors,
    const R2INTRules& globalRule,
    const sf::Text& clearText,
    const sf::Text& saveText)
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
    window->display();
}