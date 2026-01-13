#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Button {
private:
    sf::RectangleShape rect;
public:
    Button();
    void Draw(sf::RenderTarget& target);
};