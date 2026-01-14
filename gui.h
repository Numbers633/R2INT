#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <functional>

class Button : public sf::Drawable{
private:
    sf::RectangleShape rect;
    sf::Sprite icon;
    std::function<void()> callback;
    const sf::Texture* texture = nullptr;

    void UpdateIconTransform();
public:
    Button();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(rect, states);

        if (texture)
            target.draw(icon, states);
    }
    void CheckClick(const sf::Vector2f& mousePos);

    void setSize(sf::Vector2f buttonSize) { rect.setSize(buttonSize); UpdateIconTransform(); }
    void setPosition(sf::Vector2f position) { rect.setPosition(position); UpdateIconTransform(); }
    void setColor(sf::Color color) { rect.setFillColor(color); }
    void SetCallback(std::function<void()> func) { callback = func; }
    void SetIcon(const sf::Texture& tex);

    sf::Vector2f getPosition() { return rect.getPosition(); }
};

class MainGUI {
private:
    float spacing;
public:
    MainGUI(sf::Vector2u windowSize);
    void Draw(sf::RenderTarget& target);
    void Resize(sf::Vector2u windowSize);
    void HandleMouseClick(const sf::Vector2f& mousePos);

    sf::Texture playTex;
    sf::Texture pauseTex;
    sf::Texture resetTex;
    sf::Texture settingsTex;

    Button playButton;
    Button resetButton;
    Button settingsButton;
};