#include "gui.h"

Button::Button()
{
    rect.setSize({ 128.f, 128.f });          // default button size
    rect.setPosition({ 20.f, 20.f });
    rect.setFillColor(sf::Color(0, 255, 192));
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2.f);
}

void Button::Draw(sf::RenderTarget& target)
{
    target.draw(rect);
}