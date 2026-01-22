#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <functional>

class Button : public sf::Drawable{
private:
    sf::RectangleShape rect;
    sf::Sprite icon;
    std::optional<sf::Text> label;
    std::function<void()> callback;
    const sf::Texture* texture = nullptr;

    void UpdateIconTransform();
    void UpdateLabelTransform();
public:
    Button();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(rect, states);

        if (texture)
            target.draw(icon, states);

        if (label)
            target.draw(*label, states);
    }
    void CheckClick(const sf::Vector2f& mousePos);

    sf::FloatRect getBounds() const { return rect.getGlobalBounds(); }
    sf::Vector2f getPosition() const { return rect.getPosition(); }

    void setSize(sf::Vector2f buttonSize) { rect.setSize(buttonSize); UpdateIconTransform(); UpdateLabelTransform(); }
    void setPosition(sf::Vector2f position) { rect.setPosition(position); UpdateIconTransform(); UpdateLabelTransform(); }
    void setColor(sf::Color color) { rect.setFillColor(color); }
    void SetCallback(std::function<void()> func) { callback = func; }
    void SetIcon(const sf::Texture& tex);
    void SetLabel(const sf::Font& font, const std::string& text, unsigned int size);
    
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