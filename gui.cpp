#include "gui.h"
#include <iostream>

sf::Texture& GetDummyTexture()
{
    static sf::Texture dummy;
    static bool initialized = false;

    if (!initialized)
    {
        sf::Image img({ 1, 1 }, sf::Color::White);
        dummy.loadFromImage(img);
        initialized = true;
    }

    return dummy;
}           

Button::Button() : icon(GetDummyTexture())
{
    rect.setSize({ 128.f, 128.f });          // default button size
    rect.setPosition({ 20.f, 20.f });
    rect.setFillColor(sf::Color::White);
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2.f);
}

void Button::CheckClick(const sf::Vector2f& mousePos)
{
    if (rect.getGlobalBounds().contains(mousePos))
    {
        if (callback) callback(); // call the assigned function
    }
}

void Button::SetIcon(const sf::Texture& tex)
{
    texture = &tex;
    icon.setTexture(tex, true);
    UpdateIconTransform();
}
void Button::UpdateIconTransform()
{
    if (texture) {
        sf::Vector2f buttonSize = rect.getSize();
        sf::Vector2u texSize = texture->getSize();
        float scaleX = buttonSize.x / texSize.x;
        float scaleY = buttonSize.y / texSize.y;
        float scale = std::min(scaleX, scaleY);
        icon.setScale({ scale, scale });
        // Center the icon within the button
        sf::FloatRect iconBounds = icon.getLocalBounds();
        icon.setPosition({
            rect.getPosition().x + (buttonSize.x - iconBounds.size.x * scale) / 2.f,
            rect.getPosition().y + (buttonSize.y - iconBounds.size.y * scale) / 2.f }
        );
    }
}
MainGUI::MainGUI(sf::Vector2u windowSize)
{
    spacing = 16.f;
    sf::Vector2f buttonSize(64.f, 64.f);

    playTex.loadFromFile("assets\\Play.png");
    pauseTex.loadFromFile("assets\\Pause.png");

    // Bottom-right corner coordinates
    sf::Vector2f bottomRight(windowSize.x - spacing, windowSize.y - spacing);

    // Settings button (furthest right)
    settingsButton.setSize(buttonSize);
    settingsButton.setPosition({ bottomRight.x - buttonSize.x, spacing });
    settingsButton.setColor(sf::Color(0, 144, 128)); // temporary color

    // Reset button (left of Settings)
    resetButton.setSize(buttonSize);
    resetButton.setPosition(
        { bottomRight.x - buttonSize.x, bottomRight.y - buttonSize.y }
    );
    resetButton.setColor(sf::Color(96, 224, 64)); // temporary color

    // Play button (left of Reset)
    playButton.setSize(buttonSize);
    playButton.setPosition(
        { resetButton.getPosition().x - spacing - buttonSize.x,
        bottomRight.y - buttonSize.y }
    );
    playButton.setColor(sf::Color(0, 255, 128)); // temporary color
    playButton.SetIcon(playTex);

    playButton.SetCallback([]() { std::cout << "Play/Pause clicked!\n"; });
    resetButton.SetCallback([]() { std::cout << "Reset clicked!\n"; });
    settingsButton.SetCallback([]() { std::cout << "Settings clicked!\n"; });
}

void MainGUI::Resize(sf::Vector2u windowSize)
{
    sf::Vector2f buttonSize(64.f, 64.f);

    // Bottom-right corner coordinates
    sf::Vector2f bottomRight(windowSize.x - spacing, windowSize.y - spacing);

    // Settings button (furthest right)
    settingsButton.setSize(buttonSize);
    settingsButton.setPosition({ bottomRight.x - buttonSize.x, spacing });

    // Reset button (left of Settings)
    resetButton.setSize(buttonSize);
    resetButton.setPosition(
        { bottomRight.x - buttonSize.x, bottomRight.y - buttonSize.y }
    );

    // Play button (left of Reset)
    playButton.setSize(buttonSize);
    playButton.setPosition(
        { resetButton.getPosition().x - spacing - buttonSize.x,
        bottomRight.y - buttonSize.y }
    );
}

void MainGUI::Draw(sf::RenderTarget& target)
{
    target.draw(playButton);
    target.draw(resetButton);
    target.draw(settingsButton);
}

void MainGUI::HandleMouseClick(const sf::Vector2f& mousePos)
{
    playButton.CheckClick(mousePos);
    resetButton.CheckClick(mousePos);
    settingsButton.CheckClick(mousePos);
}