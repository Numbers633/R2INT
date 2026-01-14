#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <functional>

class Button : public sf::Drawable{
private:
    sf::RectangleShape rect;
    std::function<void()> callback;
public:
    Button();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override { target.draw(rect, states); }
    void CheckClick(const sf::Vector2f& mousePos);

    void setSize(sf::Vector2f buttonSize) { rect.setSize(buttonSize); }
    void setPosition(sf::Vector2f position) { rect.setPosition(position); }
    void setColor(sf::Color color) { rect.setFillColor(color); }
    void SetCallback(std::function<void()> func) { callback = func; }
    sf::Vector2f getPosition() { return rect.getPosition(); }
};

class MainGUI {
private:
    Button playButton;
    Button resetButton;
    Button settingsButton;

    float spacing;
public:
    MainGUI(sf::Vector2u windowSize);
    void Draw(sf::RenderTarget& target);
    void Resize(sf::Vector2u windowSize);
    void HandleMouseClick(const sf::Vector2f& mousePos);
};